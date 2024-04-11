#pragma once
#include "DLEngine/Math/Vec3.h"

namespace Math
{
    struct IntersectInfo
    {
        Vec3 IntersectionPoint;
        Vec3 Normal;
        float Step;
    };

    struct Ray
    {
        Vec3 Origin;
        Vec3 Direction;
    };

    struct Sphere
    {
        Vec3 Center;
        float Radius;
    };

    inline bool Intersects(const Ray& ray, const Sphere& sphere, IntersectInfo& outIntersectInfo)
    {
        const Vec3 oc = sphere.Center - ray.Origin;
        const float tc = Dot(oc, ray.Direction);

        const Vec3 ocProjectionOnRay = ray.Direction * tc;
        const float dist = Length(oc - ocProjectionOnRay);

        outIntersectInfo.Step = std::numeric_limits<float>::infinity();
        
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
       
        outIntersectInfo.Step = t;
        outIntersectInfo.IntersectionPoint = ray.Origin + ray.Direction * t;
        outIntersectInfo.Normal = Normalize(outIntersectInfo.IntersectionPoint - sphere.Center);

        return true;
    }
}
