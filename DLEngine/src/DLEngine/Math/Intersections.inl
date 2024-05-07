#pragma once
#include "DLEngine/Math/Intersections.h"

#include <stack>

#include "DLEngine/Math/Vec4.h"

#include "DLEngine/Renderer/Entity.h"
#include "DLEngine/Renderer/Mesh.h"

namespace DLEngine
{
    namespace Math
    {
        inline bool Intersects(const Ray& ray, const Sphere& sphere, IntersectInfo& outIntersectInfo)
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

        inline bool Intersects(const Ray& ray, const Plane& plane, IntersectInfo& outIntersectInfo)
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

        inline bool Intersects(const Ray& ray, const Triangle& triangle, IntersectInfo& outIntersectInfo)
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
            outIntersectInfo.Normal = Normalize(Cross(e2, e1));

            return true;
        }

        inline bool Intersects(const Ray& ray, const AABB& aabb)
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

        inline bool Intersects(const Ray& ray, const Mesh& mesh, IntersectInfo& outIntersectInfo)
        {
            const auto& nodes = mesh.GetOctree().GetNodes();
            const std::vector<Triangle>& triangles = mesh.GetTriangles();

            std::stack<uint32_t> stack;
            constexpr uint32_t rootIndex{ 0 };
            stack.push(rootIndex);

            bool intersects{ false };

            while (!stack.empty())
            {
                const uint32_t nodeIndex = stack.top();
                stack.pop();

                const auto& node = nodes[nodeIndex];
                if (!Intersects(ray, node.BoundingBox))
                    continue;

                if (node.TriangleCount > 0)
                {
                    for (uint32_t i = node.FirstTriangle; i < node.FirstTriangle + node.TriangleCount; ++i)
                    {
                        const Triangle& triangle = triangles[i];
                        if (Intersects(ray, triangle, outIntersectInfo))
                            intersects = true;
                    }
                }

                if (mesh.GetOctree().NodeHasChildren(nodeIndex))
                {
                    for (uint32_t i = 0; i < 8; ++i)
                        stack.push(node.FirstChild + i);
                }
            }

            return intersects;
        }

        inline bool Intersects(const Ray& ray, const MeshInstance& meshInstance, const Mesh& mesh, IntersectInfo& outIntersectInfo)
        {
            const Vec4 rayOriginModelSpace = Vec4{ ray.Origin, 1.0f } *meshInstance.InvTransform;
            const Vec4 rayDirModelSpace = Vec4{ ray.Direction, 0.0f } *meshInstance.InvTransform;

            Ray rayModelSpace;
            rayModelSpace.Origin = rayOriginModelSpace.xyz();
            rayModelSpace.Direction = Normalize(rayDirModelSpace.xyz());

            IntersectInfo intersectInfoModelSpace{};
            if (outIntersectInfo.T != Infinity())
            {
                const Vec4 intersectionPointModelSpace = Vec4{ outIntersectInfo.IntersectionPoint, 1.0f } *meshInstance.InvTransform;

                intersectInfoModelSpace.IntersectionPoint = intersectionPointModelSpace.xyz();
                intersectInfoModelSpace.T = Length(intersectInfoModelSpace.IntersectionPoint - rayModelSpace.Origin);
            }
            const float stepBefore = intersectInfoModelSpace.T;

            if (!Intersects(rayModelSpace, mesh, intersectInfoModelSpace))
                return false;

            if (intersectInfoModelSpace.T == stepBefore)
                return false;

            const Vec4 intersectionPointWorldSpace = Vec4{ intersectInfoModelSpace.IntersectionPoint, 1.0f } *meshInstance.Transform;
            const Vec4 normal = Normalize(Vec4{ intersectInfoModelSpace.Normal, 0.0f } *meshInstance.Transform);

            outIntersectInfo.Normal = normal.xyz();
            outIntersectInfo.IntersectionPoint = intersectionPointWorldSpace.xyz();
            outIntersectInfo.T = Length(outIntersectInfo.IntersectionPoint - ray.Origin);

            return true;
        }
    }
}
