#include "dlpch.h"
#include "MeshRegistry.h"

#include "DLEngine/Utils/RandomGenerator.h"

namespace DLEngine
{
    MeshRegistry::MeshUUID MeshRegistry::AddSubmesh(
        const Ref<Mesh>& mesh,
        uint32_t submeshIndex,
        const Ref<Material>& material,
        const Ref<Instance>& instance
    )
    {
        const auto& shader{ material->GetShader() };

        DL_ASSERT(shader == instance->GetShader(),
            "Material [{0}] and instance [{1}] are made for different shaders",
            material->GetName(), instance->GetName()
        );

        DL_ASSERT(instance->HasUniform("INSTANCE_UUID"),
            "Instance [{0}] does not have an 'INSTANCE_UUID' uniform",
            instance->GetName()
        );

        MeshBatch* meshBatch{ nullptr };
        const auto meshBatchIt{ m_MeshBatches.find(shader->GetName()) };
        if (meshBatchIt == m_MeshBatches.end())
            meshBatch = &m_MeshBatches[shader->GetName()];
        else
            meshBatch = &meshBatchIt->second;

        SubmeshBatch* submeshBatch{ nullptr };
        const auto submeshBatchIt{ meshBatch->SubmeshBatches.find(mesh) };
        if (submeshBatchIt == meshBatch->SubmeshBatches.end())
        {
            submeshBatch = &meshBatch->SubmeshBatches[mesh];
            submeshBatch->MaterialBatches.resize(mesh->GetSubmeshes().size());
        }
        else
            submeshBatch = &submeshBatchIt->second;

        DL_ASSERT(submeshIndex < submeshBatch->MaterialBatches.size(),
            "Submesh index [{0}] is out of range for mesh [{1}]",
            submeshIndex, mesh->GetName()
        );
        MaterialBatch* materialBatch{ &submeshBatch->MaterialBatches[submeshIndex] };

        InstanceBatch* instanceBatch{ nullptr };
        const auto instanceBatchIt{ materialBatch->InstanceBatches.find(material) };
        if (instanceBatchIt == materialBatch->InstanceBatches.end())
        {
            instanceBatch = &materialBatch->InstanceBatches[material];

            const auto& inputLayout{ instance->GetShader()->GetInputLayout() };
            for (const auto& [bindingPoint, inputLayoutEntry] : inputLayout)
            {
                if (inputLayoutEntry.Type == InputLayoutType::PerVertex)
                    continue;

                const auto& instanceBufferLayout{ inputLayoutEntry.Layout };
                instanceBatch->InstanceBuffers.emplace(bindingPoint, VertexBuffer::Create(instanceBufferLayout, instanceBufferLayout.GetStride()));
            }
        }
        else
            instanceBatch = &instanceBatchIt->second;

        instanceBatch->SubmeshInstances.push_back(instance);

        const auto& it{ std::find_if(std::execution::par_unseq, m_UUID_ToIntsance.begin(), m_UUID_ToIntsance.end(),
            [&instance = std::as_const(instance)](auto&& storedInstance)
            {
                return storedInstance.second == instance;
            }
        ) };
        if (it != m_UUID_ToIntsance.end())
        {
            const auto& [uuid, storedInstance]{ *it };
            
            if (m_UUID_ToMesh[uuid] != mesh)
            {
                DL_LOG_ERROR_TAG(
                    "MeshRegistry",
                    "Instance [{0}] is already assigned to a different mesh [{1}]",
                    instance->GetName(), m_UUID_ToMesh[uuid]->GetName()
                );

                instanceBatch->SubmeshInstances.pop_back();
                return 0u;
            }

            if (m_UUID_ToMaterials[uuid][submeshIndex] && m_UUID_ToMaterials[uuid][submeshIndex] != material)
            {
                DL_LOG_ERROR_TAG(
                    "MeshRegistry",
                    "Instance [{0}] is already assigned to a different material [{1}]",
                    instance->GetName(), m_UUID_ToMaterials[uuid][submeshIndex]->GetName()
                );

                instanceBatch->SubmeshInstances.pop_back();
                return 0u;
            }

            m_UUID_ToMaterials[uuid][submeshIndex] = material;

            return uuid;
        }

        const MeshUUID uuid{ RandomGenerator::GenerateRandom<MeshUUID>() };
        instance->Set("INSTANCE_UUID", Buffer{ &uuid, sizeof(MeshUUID) });
        
        m_UUID_ToMesh[uuid] = mesh;
        
        m_UUID_ToMaterials[uuid].resize(mesh->GetSubmeshes().size());
        m_UUID_ToMaterials[uuid][submeshIndex] = material;
        
        m_UUID_ToIntsance[uuid] = instance;

        return uuid;
    }

    void MeshRegistry::RemoveMesh(MeshUUID meshUUID)
    {
        if (!m_UUID_ToIntsance.contains(meshUUID))
            return;

        const Ref<Mesh>& mesh{ m_UUID_ToMesh[meshUUID] };
        const std::vector<Ref<Material>>& materials{ m_UUID_ToMaterials[meshUUID] };
        const Ref<Instance>& instance{ m_UUID_ToIntsance[meshUUID] };
        const Ref<Shader>& shader{ instance->GetShader() };
        const std::string_view shaderName{ shader->GetName() };

        MeshBatch& meshBatch{ m_MeshBatches[shaderName] };
        SubmeshBatch& submeshBatch{ meshBatch.SubmeshBatches[mesh] };

        std::for_each(submeshBatch.MaterialBatches.begin(), submeshBatch.MaterialBatches.end(),
            [&, this, meshUUID](MaterialBatch& materialBatch)
            {
                const uint32_t submeshIndex{ static_cast<uint32_t>(&materialBatch - submeshBatch.MaterialBatches.data()) };
                auto& instanceBatch{ materialBatch.InstanceBatches[materials[submeshIndex]] };
                std::erase(instanceBatch.SubmeshInstances, instance);
            }
        );

        m_UUID_ToMesh.erase(meshUUID);
        m_UUID_ToMaterials.erase(meshUUID);
        m_UUID_ToIntsance.erase(meshUUID);
    }

    void MeshRegistry::UpdateInstanceBuffers()
    {
        ClearEmptyBatches();

        for (auto& meshBatch : m_MeshBatches | std::views::values)
            for (auto& submeshBatch : meshBatch.SubmeshBatches | std::views::values)
                for (auto& materialBatch : submeshBatch.MaterialBatches)
                    for (auto& instanceBatch : materialBatch.InstanceBatches | std::views::values)
                        UpdateInstanceBuffer(instanceBatch);
    }

    void MeshRegistry::ReplaceUUID(MeshUUID oldUUID, MeshUUID newUUID)
    {
        if (!m_UUID_ToIntsance.contains(oldUUID) || oldUUID == newUUID)
            return;

        const Ref<Mesh>& mesh{ m_UUID_ToMesh[oldUUID] };
        const std::vector<Ref<Material>>& materials{ m_UUID_ToMaterials[oldUUID] };
        const Ref<Instance>& instance{ m_UUID_ToIntsance[oldUUID] };

        instance->Set("INSTANCE_UUID", Buffer{ &newUUID, sizeof(MeshUUID) });

        m_UUID_ToMesh[newUUID] = mesh;
        m_UUID_ToMaterials[newUUID] = materials;
        m_UUID_ToIntsance[newUUID] = instance;

        m_UUID_ToMesh.erase(oldUUID);
        m_UUID_ToMaterials.erase(oldUUID);
        m_UUID_ToIntsance.erase(oldUUID);
    }

    void MeshRegistry::SwapShadingGroup(MeshUUID meshUUID, const Ref<Shader>& newShader)
    {
        if (!m_UUID_ToIntsance.contains(meshUUID))
            return;

        const Ref<Mesh> mesh{ m_UUID_ToMesh[meshUUID] };
        const std::vector<Ref<Material>> materials{ m_UUID_ToMaterials[meshUUID] };
        const Ref<Instance> instance{ m_UUID_ToIntsance[meshUUID] };

        const std::string_view oldShaderName{ instance->GetShader()->GetName() };
        const std::string_view newShaderName{ newShader->GetName() };

        if (oldShaderName == newShaderName)
            return;

        const std::string& newInstanceName{ std::format("{0} to {1} Instance", instance->GetName(), newShaderName).c_str() };
        Ref<Instance> newInstance{ Instance::Copy(instance, newShader, newInstanceName) };
        std::vector<Ref<Material>> newMaterials{};
        newMaterials.reserve(materials.size());
        for (const auto& material : materials)
        {
            const std::string& newMaterialName{
                std::format("{0} to {1} Material", material->GetName(), newShaderName).c_str()
            };
            newMaterials.emplace_back(Material::Copy(material, newShader, newMaterialName));
        }

        RemoveMesh(meshUUID);

        const MeshRegistry::MeshUUID newMeshUUID{ AddSubmesh(mesh, 0u, newMaterials[0u], newInstance) };
        for (uint32_t submeshIndex{ 1 }; submeshIndex < newMaterials.size(); ++submeshIndex)
            AddSubmesh(mesh, submeshIndex, newMaterials[submeshIndex], newInstance);

        ReplaceUUID(newMeshUUID, meshUUID);
    }

    void MeshRegistry::SwapMaterial(MeshUUID meshUUID, uint32_t submeshIndex, const Ref<Material>& newMaterial)
    {
        DL_ASSERT(m_UUID_ToMaterials.contains(meshUUID),
            "Materials for mesh with UUID [{0}] do not exist",
            meshUUID
        );

        auto& materials{ m_UUID_ToMaterials.at(meshUUID) };

        DL_ASSERT(submeshIndex < materials.size(),
            "Submesh index [{0}] is out of range for mesh with UUID [{1}]",
            submeshIndex, meshUUID
        );

        const Ref<Mesh>& mesh{ m_UUID_ToMesh[meshUUID] };
        const Ref<Material>& oldMaterial{ materials[submeshIndex] };
        const Ref<Instance>& instance{ m_UUID_ToIntsance[meshUUID] };
        const std::string_view shaderName{ instance->GetShader()->GetName() };

        MeshBatch& meshBatch{ m_MeshBatches[shaderName] };
        SubmeshBatch& submeshBatch{ meshBatch.SubmeshBatches[mesh] };
        MaterialBatch& materialBatch{ submeshBatch.MaterialBatches[submeshIndex] };

        // Remove old instance
        InstanceBatch& oldInstanceBatch{ materialBatch.InstanceBatches[oldMaterial] };
        std::erase(oldInstanceBatch.SubmeshInstances, instance);

        // Add new instance
        InstanceBatch* newInstanceBatch{ nullptr };
        const auto instanceBatchIt{ materialBatch.InstanceBatches.find(newMaterial) };
        if (instanceBatchIt == materialBatch.InstanceBatches.end())
        {
            newInstanceBatch = &materialBatch.InstanceBatches[newMaterial];

            const auto& inputLayout{ instance->GetShader()->GetInputLayout() };
            for (const auto& [bindingPoint, inputLayoutEntry] : inputLayout)
            {
                if (inputLayoutEntry.Type == InputLayoutType::PerVertex)
                    continue;

                const auto& instanceBufferLayout{ inputLayoutEntry.Layout };
                newInstanceBatch->InstanceBuffers.emplace(bindingPoint, VertexBuffer::Create(instanceBufferLayout, instanceBufferLayout.GetStride()));
            }
        }
        else
            newInstanceBatch = &instanceBatchIt->second;

        newInstanceBatch->SubmeshInstances.push_back(instance);
        materials[submeshIndex] = newMaterial;
    }

    Ref<Mesh> MeshRegistry::GetMesh(MeshUUID meshUUID) const
    {
        DL_ASSERT(m_UUID_ToMesh.contains(meshUUID),
            "Mesh with UUID [{0}] does not exist",
            meshUUID
        );

        return m_UUID_ToMesh.at(meshUUID);
    }

    const Ref<Material>&  MeshRegistry::GetMaterial(MeshUUID meshUUID, uint32_t submeshIndex) const
    {
        DL_ASSERT(m_UUID_ToMaterials.contains(meshUUID),
            "Materials for mesh with UUID [{0}] do not exist",
            meshUUID
        );

        const auto& materials{ m_UUID_ToMaterials.at(meshUUID) };

        DL_ASSERT(submeshIndex < materials.size(),
            "Submesh index [{0}] is out of range for mesh with UUID [{1}]",
            submeshIndex, meshUUID
        );

        return materials[submeshIndex];
    }

    Ref<Instance> MeshRegistry::GetInstance(MeshUUID meshUUID) const
    {
        DL_ASSERT(m_UUID_ToIntsance.contains(meshUUID),
            "Instance with UUID [{0}] does not exist",
            meshUUID
        );

        return m_UUID_ToIntsance.at(meshUUID);
    }

    const MeshRegistry::MeshBatch& MeshRegistry::GetMeshBatch(std::string_view shaderName) const noexcept
    {
        const auto meshBatchIt{ m_MeshBatches.find(shaderName) };
        return meshBatchIt == m_MeshBatches.end() ? m_EmptyMeshBatch : meshBatchIt->second;
    }

    MeshRegistry::MeshBatch& MeshRegistry::GetMeshBatch(std::string_view shaderName) noexcept
    {
        auto meshBatchIt{ m_MeshBatches.find(shaderName) };
        return meshBatchIt == m_MeshBatches.end() ? m_EmptyMeshBatch : meshBatchIt->second;
    }

    void MeshRegistry::UpdateInstanceBuffer(InstanceBatch& instanceBatch)
    {
        if (instanceBatch.SubmeshInstances.empty())
            return;

        const auto& inputLayout{ instanceBatch.SubmeshInstances.front()->GetShader()->GetInputLayout() };
        std::map<uint32_t, Buffer> mapBuffers{};

        for (const auto& [bindingPoint, inputLayoutEntry] : inputLayout)
        {
            if (inputLayoutEntry.Type == InputLayoutType::PerVertex)
                continue;

            const auto& instanceBufferLayout{ inputLayoutEntry.Layout };
            const size_t instanceBufferStride{ instanceBufferLayout.GetStride() };
            const size_t requiredInstanceBufferSize{ instanceBufferStride * instanceBatch.SubmeshInstances.size() };
            if (instanceBatch.InstanceBuffers[bindingPoint]->GetSize() != requiredInstanceBufferSize)
                instanceBatch.InstanceBuffers[bindingPoint] = VertexBuffer::Create(instanceBufferLayout, requiredInstanceBufferSize);

            mapBuffers.emplace(bindingPoint, instanceBatch.InstanceBuffers[bindingPoint]->Map());
        }

        for (uint32_t submeshInstanceIndex{ 0u }; submeshInstanceIndex < instanceBatch.SubmeshInstances.size(); ++submeshInstanceIndex)
        {
            for (auto& [bindingPoint, mapBuffer] : mapBuffers)
            {
                const auto& instanceBufferLayout{ inputLayout.at(bindingPoint).Layout };
                const size_t instanceBufferStride{ instanceBufferLayout.GetStride() };
                for (const auto& bufferElement : instanceBufferLayout)
                {
                    const Buffer instanceData{ instanceBatch.SubmeshInstances[submeshInstanceIndex]->Get(bufferElement.Name) };
                    const size_t offset{ instanceBufferStride * submeshInstanceIndex + bufferElement.Offset };
                    mapBuffer.Write(instanceData.Data, instanceData.Size, offset);
                }
            }
        }

        for (const auto& [bindingPoint, instanceBuffer] : instanceBatch.InstanceBuffers)
            instanceBuffer->Unmap();
    }

    void MeshRegistry::ClearEmptyBatches()
    {
        std::erase_if(m_MeshBatches, [](auto& meshBatch)
            {
                auto& submeshBatches{ meshBatch.second.SubmeshBatches };
                std::erase_if(submeshBatches, [](auto& submeshBatch)
                    {
                        auto& materialBatches{ submeshBatch.second.MaterialBatches };
                        for (auto& materialBatch : materialBatches)
                        {
                            std::erase_if(materialBatch.InstanceBatches, [](const auto& instanceBatch)
                                {
                                    return instanceBatch.second.SubmeshInstances.empty();
                                });
                        }

                        for (const auto& materialBatch : materialBatches)
                            if (!materialBatch.InstanceBatches.empty())
                                return false;

                        return true;
                    });

                return submeshBatches.empty();
            });
    }

}