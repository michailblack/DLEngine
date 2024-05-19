#pragma once
#include "DLEngine/Core/Base.h"

#include "DLEngine/Math/Vec3.h"

#include "DLEngine/DirectX/IndexBuffer.h"
#include "DLEngine/DirectX/VertexBuffer.h"

#include "DLEngine/Mesh/Mesh.h"

namespace DLEngine
{
    class Model
    {
    public:
        struct IntersectInfo
        {
            Mesh::IntersectInfo MeshIntersectInfo;
            uint32_t MeshIndex{ 0u };
        };

        struct MeshRange
        {
            uint32_t VertexOffset;
            uint32_t IndexOffset;
            uint32_t VertexCount;
            uint32_t IndexCount;
        };

    public:
        Model(const std::string& path);

        bool Intersects(const Math::Ray& ray, Model::IntersectInfo& outIntersectInfo) const noexcept;

        uint32_t GetMeshesCount() const noexcept { return static_cast<uint32_t>(m_Meshes.size()); }

        Mesh& GetMesh(uint32_t meshIndex) noexcept;

        const Mesh& GetMesh(uint32_t meshIndex) const noexcept;
        const MeshRange& GetMeshRange(uint32_t meshIndex) const noexcept;

        const Scope<PerVertexBuffer<Mesh::Vertex>>& GetVertexBuffer() const noexcept { return m_VertexBuffer; }
        const Scope<IndexBuffer>& GetIndexBuffer() const noexcept { return m_IndexBuffer; }

        static BufferLayout GetCommonVertexBufferLayout() noexcept;

        bool operator==(const Model& other) const noexcept { return m_Name == other.m_Name; }

    private:
        std::vector<Mesh> m_Meshes;
        std::vector<MeshRange> m_Ranges;

        Scope<PerVertexBuffer<Mesh::Vertex>> m_VertexBuffer;
        Scope<IndexBuffer> m_IndexBuffer;

        std::string m_Name{};
    };
}
