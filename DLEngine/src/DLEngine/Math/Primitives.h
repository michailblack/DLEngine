#pragma once
#include "DLEngine/Math/Vec3.h"

namespace Math
{
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
}
