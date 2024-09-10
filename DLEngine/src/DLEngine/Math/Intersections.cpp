#include "dlpch.h"
#include "Intersections.h"

namespace DLEngine::Math
{
    bool Intersects(const Ray& ray, const Sphere& sphere, IntersectInfo& outIntersectInfo)
    {
        const Vec3 oc = sphere.Center - ray.Origin;
        const float tc = Dot(oc, ray.Direction);

        const Vec3 ocProjectionOnRay = ray.Direction * tc;
        const float dist = Length(oc - ocProjectionOnRay);

        if (dist > sphere.Radius)
            return false;

        const float dt = std::sqrt(sphere.Radius * sphere.Radius - dist * dist);

        // Find t for the nearest intersection point
        float t = tc - dt;
        if (t < 0.0f)
        {
            t += 2 * dt;
            if (t < 0.0f)
                return false;
        }

        if (t > outIntersectInfo.T)
            return false;

        outIntersectInfo.T = t;
        outIntersectInfo.IntersectionPoint = ray.Origin + ray.Direction * t;
        outIntersectInfo.Normal = Normalize(outIntersectInfo.IntersectionPoint - sphere.Center);

        return true;
    }

    bool Intersects(const Ray& ray, const Plane& plane, IntersectInfo& outIntersectInfo)
    {
        const float denominator = Dot(plane.Normal, ray.Direction);
        constexpr float threshold{ 1e-5f };

        if (std::abs(denominator) < threshold)
            return false;

        const float t = Dot(plane.Origin - ray.Origin, plane.Normal) / denominator;

        if (t < 0.0f || t > outIntersectInfo.T)
            return false;

        outIntersectInfo.T = t;
        outIntersectInfo.IntersectionPoint = ray.Origin + ray.Direction * t;
        outIntersectInfo.Normal = denominator > 0.0f ? -plane.Normal : plane.Normal;

        return true;
    }

    bool Intersects(const Ray& ray, const Triangle& triangle, IntersectInfo& outIntersectInfo)
    {
        const Vec3 e1 = triangle.V1 - triangle.V0;
        const Vec3 e2 = triangle.V2 - triangle.V0;
        const Vec3 s = ray.Origin - triangle.V0;
        const Vec3 p = Cross(ray.Direction, e2);
        const Vec3 q = Cross(s, e1);

        const float denominator = Dot(p, e1);
        constexpr float threshold{ 1e-5f };

        if (std::abs(denominator) < threshold)
            return false;

        const float t = Dot(q, e2) / denominator;
        const float u = Dot(p, s) / denominator;
        const float v = Dot(q, ray.Direction) / denominator;

        if (t < 0.0f || u < 0.0f || v < 0.0f || u + v > 1.0f || t > outIntersectInfo.T)
            return false;

        outIntersectInfo.T = t;
        outIntersectInfo.IntersectionPoint = ray.Origin + ray.Direction * t;
        outIntersectInfo.Normal = Normalize(Cross(e1, e2));

        return true;
    }

    bool Intersects(const Ray& ray, const AABB& aabb)
    {
        float maxS = -std::numeric_limits<float>::max();
        float minT = std::numeric_limits<float>::max();

        float s, t;
        const float recipX{ 1.0f / ray.Direction.x };
        if (recipX >= 0.0f)
        {
            s = (aabb.Min.x - ray.Origin.x) * recipX;
            t = (aabb.Max.x - ray.Origin.x) * recipX;
        }
        else
        {
            s = (aabb.Max.x - ray.Origin.x) * recipX;
            t = (aabb.Min.x - ray.Origin.x) * recipX;
        }

        maxS = std::max(maxS, s);
        minT = std::min(minT, t);

        if (maxS > minT)
            return false;

        maxS = -std::numeric_limits<float>::max();
        minT = std::numeric_limits<float>::max();

        const float recipY{ 1.0f / ray.Direction.y };
        if (recipY >= 0.0f)
        {
            s = (aabb.Min.y - ray.Origin.y) * recipY;
            t = (aabb.Max.y - ray.Origin.y) * recipY;
        }
        else
        {
            s = (aabb.Max.y - ray.Origin.y) * recipY;
            t = (aabb.Min.y - ray.Origin.y) * recipY;
        }

        maxS = std::max(maxS, s);
        minT = std::min(minT, t);

        if (maxS > minT)
            return false;

        maxS = -std::numeric_limits<float>::max();
        minT = std::numeric_limits<float>::max();

        const float recipZ{ 1.0f / ray.Direction.z };
        if (recipZ >= 0.0f)
        {
            s = (aabb.Min.z - ray.Origin.z) * recipZ;
            t = (aabb.Max.z - ray.Origin.z) * recipZ;
        }
        else
        {
            s = (aabb.Max.z - ray.Origin.z) * recipZ;
            t = (aabb.Min.z - ray.Origin.z) * recipZ;
        }

        maxS = std::max(maxS, s);
        minT = std::min(minT, t);

        if (maxS > minT)
            return false;

        return true;
    }

    bool Intersects(const Ray& ray, const TriangleOctree& octree, const Submesh& targetSubmesh, uint32_t& outTriangleIndex)
    {
        bool intersects{ false };

        constexpr uint32_t rootIndex{ 0u };
        std::stack<uint32_t> stack;
        stack.push(rootIndex);

        const auto& triangles{ targetSubmesh.GetTriangles() };
        const auto& vertices{ targetSubmesh.GetVertices() };

        const auto& octreeNodes{ octree.GetNodes() };
        const auto& octreeTriangleIndices{ octree.GetTriangleIndices() };

        Math::IntersectInfo intersectInfo{};

        while (!stack.empty())
        {
            const uint32_t nodeIndex{ stack.top() };
            stack.pop();

            const TriangleOctree::OctreeNode& node{ octreeNodes[nodeIndex] };

            if (!Math::Intersects(ray, node.BoundingBox))
                continue;

            if (node.TriangleCount > 0u)
            {
                for (uint32_t i{ node.FirstTriangle }; i < node.FirstTriangle + node.TriangleCount; ++i)
                {
                    const auto& triangle{ triangles[octreeTriangleIndices[i]] };
                    Math::Triangle triangleToCheck{
                        .V0 = vertices[triangle.Indices[0]].Position,
                        .V1 = vertices[triangle.Indices[1]].Position,
                        .V2 = vertices[triangle.Indices[2]].Position
                    };

                    if (Math::Intersects(ray, triangleToCheck, intersectInfo))
                    {
                        outTriangleIndex = octreeTriangleIndices[i];
                        intersects = true;
                    }
                }
            }

            if (octree.NodeHasChildren(nodeIndex))
            {
                for (uint32_t i{ 0u }; i < 8u; ++i)
                    stack.push(node.FirstChild + i);
            }
        }

        return intersects;
    }

    bool Intersects(const Ray& ray, const Submesh& submesh, Submesh::IntersectInfo& outIntersectInfo)
    {
        if (!Math::Intersects(ray, submesh.GetBoundingBox()))
            return false;

        uint32_t intersectedTriangleIndex{ 0u };
        if (Math::Intersects(ray, submesh.GetOctree(), submesh, intersectedTriangleIndex))
        {
            const auto& triangles{ submesh.GetTriangles() };
            const auto& vertices{ submesh.GetVertices() };
            Math::Triangle triangleToCheck{
                .V0 = vertices[triangles[intersectedTriangleIndex].Indices[0]].Position,
                .V1 = vertices[triangles[intersectedTriangleIndex].Indices[1]].Position,
                .V2 = vertices[triangles[intersectedTriangleIndex].Indices[2]].Position
            };

            Math::IntersectInfo triangleIntersectInfo{};
            DL_VERIFY(Math::Intersects(ray, triangleToCheck, triangleIntersectInfo));

            if (triangleIntersectInfo.T < outIntersectInfo.TriangleIntersectInfo.T)
            {
                outIntersectInfo.TriangleIntersectInfo = triangleIntersectInfo;
                outIntersectInfo.TriangleIndex = intersectedTriangleIndex;
                return true;
            }
        }

        return false;
    }

    bool Intersects(const Ray& ray, const MeshRegistry& meshRegistry, MeshRegistry::IntersectInfo& outIntersectInfo)
    {
        bool intersects{ false };
        for (const auto& meshBatch : meshRegistry | std::views::values)
        {
            for (const auto& [mesh, submeshBatch] : meshBatch.SubmeshBatches)
            {
                for (uint32_t submeshIndex{ 0u }; submeshIndex < submeshBatch.MaterialBatches.size(); ++submeshIndex)
                {
                    const auto& submesh{ mesh->GetSubmeshes()[submeshIndex] };
                    for (const auto& [material, instanceBatch] : submeshBatch.MaterialBatches[submeshIndex].InstanceBatches)
                    {
                        for (const auto& instance : instanceBatch.SubmeshInstances)
                        {
                            if (!instance->HasUniform("TRANSFORM"))
                                continue;

                            const auto& meshToWorld{ instance->Get<Math::Mat4x4>("TRANSFORM") };
                            const auto& worldToMesh{ Math::Mat4x4::Inverse(meshToWorld) };

                            if (!Math::Intersects(ray, AABBToSpace(submesh.GetBoundingBox(), meshToWorld)))
                                continue;

                            const Math::Ray rayMeshSpace{ RayToSpace(ray, worldToMesh) };

                            Submesh::IntersectInfo meshSpaceIntersectInfo{};
                            if (outIntersectInfo.SubmeshIntersectInfo.TriangleIntersectInfo.T != Math::Numeric::Inf)
                            {
                                meshSpaceIntersectInfo.TriangleIntersectInfo.IntersectionPoint = Math::PointToSpace(
                                    outIntersectInfo.SubmeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint,
                                    worldToMesh
                                );
                                meshSpaceIntersectInfo.TriangleIntersectInfo.T = Math::Length(
                                    meshSpaceIntersectInfo.TriangleIntersectInfo.IntersectionPoint - rayMeshSpace.Origin
                                );
                            }

                            if (Math::Intersects(rayMeshSpace, submesh, meshSpaceIntersectInfo))
                            {
                                outIntersectInfo.SubmeshIntersectInfo.TriangleIndex = meshSpaceIntersectInfo.TriangleIndex;
                                outIntersectInfo.SubmeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint = Math::PointToSpace(
                                    meshSpaceIntersectInfo.TriangleIntersectInfo.IntersectionPoint,
                                    meshToWorld
                                );
                                outIntersectInfo.SubmeshIntersectInfo.TriangleIntersectInfo.Normal = Math::Normalize(Math::DirectionToSpace(
                                    meshSpaceIntersectInfo.TriangleIntersectInfo.Normal,
                                    meshToWorld
                                ));
                                outIntersectInfo.SubmeshIntersectInfo.TriangleIntersectInfo.T = Math::Length(
                                    outIntersectInfo.SubmeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint - ray.Origin
                                );

                                outIntersectInfo.UUID = instance->Get<uint64_t>("INSTANCE_UUID");

                                intersects = true;
                            }
                        }
                    }
                }
            }
        }

        return intersects;
    }

}