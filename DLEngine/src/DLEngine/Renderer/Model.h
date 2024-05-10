#pragma once
#include "DLEngine/DirectX/IndexBuffer.h"
#include "DLEngine/DirectX/VertexBuffer.h"

#include "DLEngine/Renderer/Mesh.h"

namespace DLEngine
{
    class Model
    {
    public:
        struct MeshRange
        {
            uint32_t VertexOffset;
            uint32_t IndexOffset;
            uint32_t VertexCount;
            uint32_t IndexCount;
        };

    public:
        Model(const std::string& path);

        uint32_t GetMeshesCount() const noexcept { return static_cast<uint32_t>(m_Meshes.size()); }

        Mesh& GetMesh(uint32_t meshIndex) noexcept { return m_Meshes[meshIndex]; }

        const Mesh& GetMesh(uint32_t meshIndex) const noexcept { return m_Meshes[meshIndex]; }
        const MeshRange& GetMeshRange(uint32_t meshIndex) const noexcept { return m_Ranges[meshIndex]; }

        const Scope<PerVertexBuffer<Mesh::Vertex>>& GetVertexBuffer() const noexcept { return m_VertexBuffer; }
        const Scope<IndexBuffer>& GetIndexBuffer() const noexcept { return m_IndexBuffer; }

    private:
        std::vector<Mesh> m_Meshes;
        std::vector<MeshRange> m_Ranges;

        Scope<PerVertexBuffer<Mesh::Vertex>> m_VertexBuffer;
        Scope<IndexBuffer> m_IndexBuffer;

        std::string m_Name{};
        Math::AABB m_BoundingBox{};
    };
}
