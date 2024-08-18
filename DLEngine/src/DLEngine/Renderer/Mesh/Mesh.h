#pragma once
#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Primitives.h"
#include "DLEngine/Math/Vec2.h"

#include "DLEngine/Renderer/Mesh/TriangleOctree.h"

#include "DLEngine/Renderer/IndexBuffer.h"
#include "DLEngine/Renderer/VertexBuffer.h"

#include <filesystem>

namespace DLEngine
{
    class Submesh
    {
    public:
        struct IntersectInfo
        {
            Math::IntersectInfo TriangleIntersectInfo;
            uint32_t TriangleIndex;
        };

        struct Vertex
        {
            Math::Vec3 Position;
            Math::Vec3 Normal;
            Math::Vec3 Tangent;
            Math::Vec3 Bitangent;
            Math::Vec2 TexCoords;
        };

        struct Triangle
        {
            uint32_t Indices[3];
        };

    public:
        void UpdateOctree() { m_Octree.Rebuild(*this); }

        const std::vector<Vertex>& GetVertices() const noexcept { return m_Vertices; }
        const std::vector<Triangle>& GetTriangles() const noexcept { return m_Triangles; }
        const std::vector<Math::Mat4x4>& GetInstances() const noexcept { return m_Instances; }
        const std::vector<Math::Mat4x4>& GetInvInstances() const noexcept { return m_InvInstances; }

        const TriangleOctree& GetOctree() const noexcept { return m_Octree; }

        const Math::AABB& GetBoundingBox() const noexcept { return m_BoundingBox; }

    private:
        TriangleOctree m_Octree;

        std::string m_Name;
        
        std::vector<Vertex> m_Vertices;
        std::vector<Triangle> m_Triangles;
        std::vector<Math::Mat4x4> m_Instances;
        std::vector<Math::Mat4x4> m_InvInstances;

        Math::AABB m_BoundingBox;

    private:
        friend class Mesh;
    };

    class Mesh
    {
    public:
        struct Range
        {
            uint32_t VertexOffset;
            uint32_t VertexCount;
            uint32_t IndexOffset;
            uint32_t IndexCount;
        };

    public:
        Mesh() noexcept = default;
        Mesh(const std::filesystem::path& path) noexcept;

        const std::string& GetName() const noexcept { return m_Name; }

        const std::vector<Submesh>& GetSubmeshes() const noexcept { return m_Submeshes; }
        const std::vector<Range>& GetRanges() const noexcept { return m_Ranges; }

        const Math::AABB& GetBoundingBox() const noexcept { return m_BoundingBox; }

        const Ref<VertexBuffer>& GetVertexBuffer() const noexcept { return m_VertexBuffer; }
        const Ref<IndexBuffer>& GetIndexBuffer() const noexcept { return m_IndexBuffer; }

        static VertexBufferLayout GetCommonVertexBufferLayout() noexcept;

        static const std::filesystem::path GetMeshDirectoryPath() noexcept;

        static Ref<Mesh> CreateUnitSphere();

    private:
        void LoadFromFile(const std::filesystem::path& path);

    private:
        std::string m_Name;

        std::vector<Submesh> m_Submeshes;
        std::vector<Range> m_Ranges;

        Math::AABB m_BoundingBox;

        Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;
    };

    class MeshLibrary
    {
    public:
        void Init();

        void Add(const Ref<Mesh>& mesh);
        Ref<Mesh> Load(const std::filesystem::path& path);
        Ref<Mesh> Get(const std::string& path);

    private:
        std::unordered_map<std::string, Ref<Mesh>> m_Meshes{};
    };
}