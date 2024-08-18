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

        MaterialBatch* materialBatch{ &submeshBatch->MaterialBatches[submeshIndex] };

        InstanceBatch* instanceBatch{ nullptr };
        const auto instanceBatchIt{ materialBatch->InstanceBatches.find(material) };
        if (instanceBatchIt == materialBatch->InstanceBatches.end())
            instanceBatch = &materialBatch->InstanceBatches[material];
        else
            instanceBatch = &instanceBatchIt->second;

        instanceBatch->SubmeshInstances.push_back(instance);
    }

    void MeshRegistry::UpdateInstanceBuffers()
    {
        for (auto& meshBatch : m_MeshBatches | std::views::values)
            for (auto& submeshBatch : meshBatch.SubmeshBatches | std::views::values)
                for (auto& materialBatch : submeshBatch.MaterialBatches)
                    for (auto& instanceBatch : materialBatch.InstanceBatches | std::views::values)
                        UpdateInstanceBuffer(instanceBatch);
    }

    const MeshRegistry::MeshBatch& MeshRegistry::GetMeshBatch(std::string_view shaderName) const
    {
        DL_ASSERT(m_MeshBatches.contains(shaderName), "Mesh registry does not contain mesh batch for shader [{0}]", shaderName);
        return m_MeshBatches.at(shaderName);
    }

    void MeshRegistry::UpdateInstanceBuffer(InstanceBatch& instanceBatch)
    {
        if (instanceBatch.SubmeshInstances.empty())
            return;

        const auto& instanceBufferLayout{ instanceBatch.SubmeshInstances.front()->GetShader()->GetInstanceLayout() };
        size_t instanceBufferSize{ instanceBufferLayout.GetStride() };
        instanceBatch.InstanceBuffer = VertexBuffer::Create(instanceBufferLayout, instanceBufferSize * instanceBatch.SubmeshInstances.size());

        Buffer buffer{ instanceBatch.InstanceBuffer->Map() };
        for (size_t i{ 0 }; i < instanceBatch.SubmeshInstances.size(); ++i)
            buffer.Write(instanceBatch.SubmeshInstances[i]->GetInstanceData().Data, instanceBufferSize, instanceBufferSize * i);
        instanceBatch.InstanceBuffer->Unmap();
    }

}