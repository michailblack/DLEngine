#pragma once
#include "DLEngine/Math/Primitives.h"
#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Vec2.h"
#include "DLEngine/Math/Vec3.h"

#include "DLEngine/Mesh/TriangleOctree.h"

namespace DLEngine
{
    struct ModelIntersectInfo;

    class Mesh
    {
        friend class Model;
    public:
        struct IntersectInfo
        {
            Math::IntersectInfo TriangleIntersectInfo;
            uint32_t TriangleIndex{ 0u };
            uint32_t InstanceIndex{ 0u };
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
        Mesh() noexcept;

        void UpdateOctree() { m_Octree.Build(); }

        bool Intersects(const Math::Ray& ray, Mesh::IntersectInfo& outIntersectInfo) const noexcept;

        const Vertex& GetVertex(uint32_t vertexIndex) const noexcept;
        
        const std::vector<Triangle>& GetTriangles() const noexcept { return m_Triangles; }

        uint32_t GetInstanceCount() const noexcept { return static_cast<uint32_t>(m_Instances.size()); }

        Math::Mat4x4& GetInstance(uint32_t instanceIndex) noexcept;
        Math::Mat4x4& GetInvInstance(uint32_t instanceIndex) noexcept;

        const Math::Mat4x4& GetInstance(uint32_t instanceIndex) const noexcept;
        const Math::Mat4x4& GetInvInstance(uint32_t instanceIndex) const noexcept;

        const Math::AABB& GetBoundingBox() const noexcept { return m_BoundingBox; }

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<Triangle> m_Triangles;
        std::vector<Math::Mat4x4> m_Instances;
        std::vector<Math::Mat4x4> m_InvInstances;
        
        TriangleOctree m_Octree;

        std::string m_Name;
        Math::AABB m_BoundingBox;
    };
}
