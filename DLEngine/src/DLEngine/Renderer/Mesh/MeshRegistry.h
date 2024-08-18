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
        struct SubmeshID
        {
            Ref<Mesh> Mesh;
            Ref<Material> Material;
            Ref<Instance> Instance;
            uint32_t SubmeshIndex{ static_cast<uint32_t>(-1) };
        };

        struct IntersectInfo
        {
            Submesh::IntersectInfo SubmeshIntersectInfo;
            SubmeshID SubmeshID;
        };

        struct InstanceBatch
        {
            std::vector<Ref<Instance>> SubmeshInstances;
            Ref<VertexBuffer> InstanceBuffer;
        };

        struct MaterialBatch
        {
            std::unordered_map<Ref<Material>, InstanceBatch> InstanceBatches;
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
        void AddSubmesh(const Ref<Mesh>& mesh, uint32_t submeshIndex, const Ref<Material>& material, const Ref<Instance>& instance);

        void UpdateInstanceBuffers();

        const MeshBatch& GetMeshBatch(std::string_view shaderName) const;

        [[nodiscard]] std::unordered_map<std::string_view, MeshBatch>::const_iterator begin() const noexcept { return m_MeshBatches.begin(); }
        [[nodiscard]] std::unordered_map<std::string_view, MeshBatch>::const_iterator end() const noexcept { return m_MeshBatches.end(); }

    private:
        void UpdateInstanceBuffer(InstanceBatch& instanceBatch);

    private:
        std::unordered_map<std::string_view, MeshBatch> m_MeshBatches;
    };
}