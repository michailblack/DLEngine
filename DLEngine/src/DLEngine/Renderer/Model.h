#pragma once
#include "DLEngine/DirectX/IndexBuffer.h"

#include "DLEngine/Renderer/Mesh.h"

namespace DLEngine
{
    template <typename T>
    class PerVertexBuffer;

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

    protected:
        std::vector<Mesh> m_Meshes;
        std::vector<MeshRange> m_Ranges;

        Scope<PerVertexBuffer<Mesh::Vertex>> m_VertexBuffer;
        Scope<IndexBuffer> m_IndexBuffer;

    private:
        std::string m_Name{};
        Math::AABB m_BoundingBox{};
    };
}
