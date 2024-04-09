#pragma once
#include <optional>

#include "DLEngine/Math/Vec.h"

namespace Math
{
    struct IntersectInfo
    {
        Vec3f IntersectionPoint;
        Vec3f Normal;
        float Step;
    };

    struct Ray
    {
        Vec3f Origin;
        Vec3f Direction;
    };

    struct Sphere
    {
        Vec3f Center;
        float Radius;
    };

    inline std::optional<IntersectInfo> Intersects(const Ray& ray, const Sphere& sphere)
    {
        const Vec3f oc = sphere.Center - ray.Origin;
        const float tc = Dot(oc, ray.Direction);
        const Vec3f ocProjectionOnRay = ray.Direction * tc;
        const float dist = Length(oc - ocProjectionOnRay);
        
        if (dist > sphere.Radius)
            return std::nullopt;

        const float dt = std::sqrt(sphere.Radius * sphere.Radius - dist * dist);

        // Find t for the nearest intersection point
        float t = tc - dt;
        if (t < 0.0f)
        {
            t += 2 * dt;
            if (t < 0.0f)
                return std::nullopt;
        }

        IntersectInfo info;
        info.Step = t;
        info.IntersectionPoint = ray.Origin + ray.Direction * info.Step;
        info.Normal = Normalize(info.IntersectionPoint - sphere.Center);

        return info;
    }
}
