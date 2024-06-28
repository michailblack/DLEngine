#pragma once
#include "DLEngine/Math/Primitives.h"

namespace DLEngine::Math
{
    inline bool Intersects(const Ray& ray, const Sphere& sphere, IntersectInfo& outIntersectInfo);
    inline bool Intersects(const Ray& ray, const Plane& plane, IntersectInfo& outIntersectInfo);
    inline bool Intersects(const Ray& ray, const Triangle& triangle, IntersectInfo& outIntersectInfo);
    inline bool Intersects(const Ray& ray, const AABB& aabb);
}

#include "Intersections.inl"
