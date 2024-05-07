#pragma once
#include "DLEngine/Math/Primitives.h"

namespace DLEngine
{
    class Mesh;
    struct MeshInstance;
    class TriangleOctree;

    namespace Math
    {
        inline bool Intersects(const Ray& ray, const Sphere& sphere, IntersectInfo& outIntersectInfo);
        inline bool Intersects(const Ray& ray, const Plane& plane, IntersectInfo& outIntersectInfo);
        inline bool Intersects(const Ray& ray, const Triangle& triangle, IntersectInfo& outIntersectInfo);
        inline bool Intersects(const Ray& ray, const AABB& aabb);
        inline bool Intersects(const Ray& ray, const Mesh& mesh, IntersectInfo& outIntersectInfo);
        inline bool Intersects(const Ray& ray, const MeshInstance& meshInstance, const Mesh& mesh, IntersectInfo& outIntersectInfo);
    }
}

#include "Intersections.inl"
