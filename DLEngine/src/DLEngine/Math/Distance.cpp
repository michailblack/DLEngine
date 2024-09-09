#include "dlpch.h"
#include "Distance.h"

namespace DLEngine::Math
{
    float Distance(const Vec3& point, const Plane& plane)
    {
        return std::abs(Dot(plane.Normal, point - plane.Origin));
    }
}