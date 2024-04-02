#pragma once
#include "DLEngine/Math/Vec.h"

namespace Math
{
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

    inline bool Intersects(const Ray& ray, const Sphere& sphere)
    {
        const Vec3f oc = sphere.Center - ray.Origin;
        const Vec3f ocProjectionOnRay = ray.Direction * Dot(oc, ray.Direction);
        const Vec3f dist = oc - ocProjectionOnRay;
        return Dot(dist, dist) <= sphere.Radius * sphere.Radius;
    }
}
