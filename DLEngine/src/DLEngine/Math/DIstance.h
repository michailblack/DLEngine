#pragma once
#include "DLEngine/Math/Primitives.h"

namespace DLEngine
{
    namespace Math
    {
        inline float Distance(const Vec3& point, const Plane& plane);
    }
}

#include "Distance.inl"
