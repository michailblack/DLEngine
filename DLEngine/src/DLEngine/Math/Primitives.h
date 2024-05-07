#pragma once
#include "DLEngine/Math/Math.h"
#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
    namespace Math
    {
        struct IntersectInfo
        {
            Vec3 IntersectionPoint;
            Vec3 Normal;
            float T{ Infinity() };
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

        struct Triangle
        {
            Vec3 V0;
            Vec3 V1;
            Vec3 V2;
        };

        struct Plane
        {
            Vec3 Origin;
            Vec3 Normal;
        };

        struct AABB
        {
            Vec3 Min;
            Vec3 Max;
        };
    }

    namespace Math
    {
        inline float Distance(const Vec3& point, const Plane& plane)
        {
            return Dot(plane.Normal, point - plane.Origin);
        }
    }
}
