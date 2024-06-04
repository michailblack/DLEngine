#pragma once
#include "DLEngine/Core/Base.h"

#include "DLEngine/Math/Vec3.h"

#include "DLEngine/DirectX/IndexBuffer.h"
#include "DLEngine/DirectX/VertexBuffer.h"

#include "DLEngine/Systems/Mesh/Mesh.h"

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
        Model(const std::wstring& path);

        uint32_t GetMeshesCount() const noexcept { return static_cast<uint32_t>(m_Meshes.size()); }

        const Mesh& GetMesh(uint32_t meshIndex) const noexcept;
        const MeshRange& GetMeshRange(uint32_t meshIndex) const noexcept;

        const VertexBuffer<Mesh::Vertex, VertexBufferUsage::Immutable>& GetVertexBuffer() const noexcept { return m_VertexBuffer; }
        const IndexBuffer& GetIndexBuffer() const noexcept { return m_IndexBuffer; }

        static BufferLayout GetCommonVertexBufferLayout() noexcept;

        bool operator==(const Model& other) const noexcept { return m_Name == other.m_Name; }

    private:
        std::vector<Mesh> m_Meshes;
        std::vector<MeshRange> m_Ranges;

        VertexBuffer<Mesh::Vertex, VertexBufferUsage::Immutable> m_VertexBuffer;
        IndexBuffer m_IndexBuffer;

        std::wstring m_Name{};
    };
}
