#include "dlpch.h"
#include "MeshRegistry.h"

namespace DLEngine
{
    void MeshRegistry::AddSubmesh(
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
    }

    void MeshRegistry::RemoveSubmesh(const Ref<Mesh>& mesh, uint32_t submeshIndex, const Ref<Material>& material, const Ref<Instance>& instance)
    {
        const auto& shader{ material->GetShader() };

        DL_ASSERT(shader == instance->GetShader(),
            "Material [{0}] and instance [{1}] are made for different shaders",
            material->GetName(), instance->GetName()
        );

        MeshBatch* meshBatch{ nullptr };
        const auto meshBatchIt{ m_MeshBatches.find(shader->GetName()) };
        if (meshBatchIt == m_MeshBatches.end())
        {
            DL_LOG_WARN_TAG("MeshRegistry", "Trying to remove submesh from non-existing mesh batch [{0}]", shader->GetName());
            return;
        }
        else
            meshBatch = &meshBatchIt->second;

        SubmeshBatch* submeshBatch{ nullptr };
        const auto submeshBatchIt{ meshBatch->SubmeshBatches.find(mesh) };
        if (submeshBatchIt == meshBatch->SubmeshBatches.end())
        {
            DL_LOG_WARN_TAG("MeshRegistry", "Trying to remove submesh from non-existing submesh batch [{0}]", mesh->GetName());
            return;
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
            DL_LOG_WARN_TAG("MeshRegistry", "Trying to remove submesh from non-existing instance batch [{0}]", material->GetName());
            return;
        }
        else
            instanceBatch = &instanceBatchIt->second;

        const auto removedInstanceCount{ std::erase(instanceBatch->SubmeshInstances, instance) };
        DL_ASSERT(removedInstanceCount == 0u || removedInstanceCount == 1u,
            "Removed more than one instance from instance batch [{0}]",
            material->GetName()
        );
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