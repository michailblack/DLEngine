#include "dlpch.h"
#include "Mesh.h"

#include "DLEngine/Math/Intersections.h"

#include "DLEngine/Mesh/Model.h"

namespace DLEngine
{

    Mesh::Mesh() noexcept
        : m_Octree(*this)
    {

    }

    bool Mesh::Intersects(const Math::Ray& ray, Mesh::IntersectInfo& outIntersectInfo) const noexcept
    {
        bool intersects{ false };
        for (uint32_t instanceIndex{ 0u }; instanceIndex < m_Instances.size(); ++instanceIndex)
        {
            const Math::Mat4x4& meshToModel{ m_Instances[instanceIndex] };
            const Math::Mat4x4& modelToMesh{ m_InvInstances[instanceIndex] };

            Math::Ray meshSpaceRay{
                Math::Vec4{ Math::Vec4{ ray.Origin, 1.0f } * modelToMesh }.xyz(),
                Math::Normalize(Math::Vec4{ Math::Vec4{ ray.Direction, 0.0f } * modelToMesh }.xyz())
            };

            uint32_t triangleIndex{ 0u };
            if (m_Octree.Intersects(meshSpaceRay, triangleIndex))
            {
                Math::IntersectInfo triangleIntersectInfo{};
                Math::Triangle triangle{
                    Math::Vec4{ Math::Vec4{ m_Vertices[m_Triangles[triangleIndex].Indices[0]].Position, 1.0f } * meshToModel }.xyz(),
                    Math::Vec4{ Math::Vec4{ m_Vertices[m_Triangles[triangleIndex].Indices[1]].Position, 1.0f } * meshToModel }.xyz(),
                    Math::Vec4{ Math::Vec4{ m_Vertices[m_Triangles[triangleIndex].Indices[2]].Position, 1.0f } * meshToModel }.xyz()
                };

                DL_ASSERT_EXPR(Math::Intersects(ray, triangle, triangleIntersectInfo));

                if (triangleIntersectInfo.T < outIntersectInfo.TriangleIntersectInfo.T)
                {
                    outIntersectInfo.TriangleIntersectInfo.T = triangleIntersectInfo.T;

                    outIntersectInfo.TriangleIntersectInfo.IntersectionPoint = ray.Origin + ray.Direction * outIntersectInfo.TriangleIntersectInfo.T;
                    outIntersectInfo.TriangleIntersectInfo.Normal = triangleIntersectInfo.Normal;

                    outIntersectInfo.TriangleIndex = triangleIndex;
                    outIntersectInfo.InstanceIndex = instanceIndex;

                    intersects = true;
                }
            }
        }

        return intersects;
    }

    const Mesh::Vertex& Mesh::GetVertex(uint32_t vertexIndex) const noexcept
    {
        DL_ASSERT_NOINFO(vertexIndex < m_Vertices.size());
        return m_Vertices[vertexIndex];
    }

    Math::Mat4x4& Mesh::GetInstance(uint32_t instanceIndex) noexcept
    {
        DL_ASSERT_NOINFO(instanceIndex < m_Instances.size());
        return m_Instances[instanceIndex];
    }

    const Math::Mat4x4& Mesh::GetInstance(uint32_t instanceIndex) const noexcept
    {
        DL_ASSERT_NOINFO(instanceIndex < m_Instances.size());
        return m_Instances[instanceIndex];
    }

    Math::Mat4x4& Mesh::GetInvInstance(uint32_t instanceIndex) noexcept
    {
        DL_ASSERT_NOINFO(instanceIndex < m_Instances.size());
        return m_InvInstances[instanceIndex];
    }

    const Math::Mat4x4& Mesh::GetInvInstance(uint32_t instanceIndex) const noexcept
    {
        DL_ASSERT_NOINFO(instanceIndex < m_Instances.size());
        return m_InvInstances[instanceIndex];
    }

}
