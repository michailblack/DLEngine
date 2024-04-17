#pragma once
#include "DLEngine/Math/Math.h"
#include "DLEngine/Math/Primitives.h"

namespace Math
{
    struct IntersectInfo
    {
        Vec3 IntersectionPoint { 0.0f, 0.0f, 0.0f };
        Vec3 Normal { 0.0f, 0.0f, 0.0f };
        float Step { Infinity() };
    };

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

        if (t > outIntersectInfo.Step)
            return false;
       
        outIntersectInfo.Step = t;
        outIntersectInfo.IntersectionPoint = ray.Origin + ray.Direction * t;
        outIntersectInfo.Normal = Normalize(outIntersectInfo.IntersectionPoint - sphere.Center);

        return true;
    }

    inline bool Intersects(const Ray& ray, const Plane& plane, IntersectInfo& outIntersectInfo)
    {
        const float denominator = Dot(plane.Normal, ray.Direction);
        constexpr float threshold { 1e-5f };

        if (std::abs(denominator) < threshold)
            return false;

        const float t = Dot(plane.Origin - ray.Origin, plane.Normal) / denominator;

        if (t < 0.0f || t > outIntersectInfo.Step)
            return false;

        outIntersectInfo.Step = t;
        outIntersectInfo.IntersectionPoint = ray.Origin + ray.Direction * t;
        outIntersectInfo.Normal = plane.Normal;

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
        constexpr float threshold { 1e-5f };

        if (std::abs(denominator) < threshold)
            return false;

        const float t = Dot(q, e2) / denominator;
        const float u = Dot(p, s) / denominator;
        const float v = Dot(q, ray.Direction) / denominator;

        if (t < 0.0f || u < 0.0f || v < 0.0f || u + v > 1.0f || t > outIntersectInfo.Step)
            return false;

        outIntersectInfo.Step = t;
        outIntersectInfo.IntersectionPoint = ray.Origin + ray.Direction * t;
        outIntersectInfo.Normal = Normalize(Cross(e1, e2));

        return true;
    }
}
