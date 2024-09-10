#pragma once
#include "DLEngine/Renderer/Mesh/Mesh.h"

#include "DLEngine/Renderer/Instance.h"
#include "DLEngine/Renderer/Material.h"
#include "DLEngine/Renderer/VertexBuffer.h"

namespace DLEngine
{
    class MeshRegistry
    {
    public:
        using MeshUUID = uint64_t;

    public:
        struct IntersectInfo
        {
            Submesh::IntersectInfo SubmeshIntersectInfo;
            MeshUUID UUID;
        };

        struct InstanceBatch
        {
            std::vector<Ref<Instance>> SubmeshInstances;
            std::map<uint32_t, Ref<VertexBuffer>> InstanceBuffers;
        };

        struct MaterialBatch
        {
            std::unordered_map<Ref<Material>, InstanceBatch, MaterialHash, MaterialEqual> InstanceBatches;
        };

        struct SubmeshBatch
        {
            std::vector<MaterialBatch> MaterialBatches;
        };

        struct MeshBatch
        {
            std::unordered_map<Ref<Mesh>, SubmeshBatch> SubmeshBatches;
        };

    public:
        MeshUUID AddSubmesh(const Ref<Mesh>& mesh, uint32_t submeshIndex, const Ref<Material>& material, const Ref<Instance>& instance);
        void RemoveSubmesh(MeshUUID submeshUUID);

        void UpdateInstanceBuffers();

        bool HasInstance(MeshUUID submeshUUID) const { return m_UUID_ToIntsance.contains(submeshUUID); }
        Ref<Instance> GetInstance(MeshUUID submeshUUID) const;

        MeshBatch& GetMeshBatch(std::string_view shaderName) noexcept;
        const MeshBatch& GetMeshBatch(std::string_view shaderName) const noexcept;

        [[nodiscard]] std::unordered_map<std::string_view, MeshBatch>::const_iterator begin() const noexcept { return m_MeshBatches.begin(); }
        [[nodiscard]] std::unordered_map<std::string_view, MeshBatch>::const_iterator end() const noexcept { return m_MeshBatches.end(); }

    private:
        void UpdateInstanceBuffer(InstanceBatch& instanceBatch);
        void ClearEmptyBatches();

    private:
        std::unordered_map<std::string_view, MeshBatch> m_MeshBatches;

        std::unordered_map<MeshUUID, Ref<Instance>> m_UUID_ToIntsance;
        std::unordered_map<Ref<Instance>, MeshUUID> m_InstanceToUUID;

        MeshBatch m_EmptyMeshBatch;
    };
}