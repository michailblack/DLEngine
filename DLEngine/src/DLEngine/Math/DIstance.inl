#pragma once

namespace DLEngine
{
    inline float Distance(const Vec3& point, const Plane& plane)
    {
        return std::abs(Dot(plane.Normal, point - plane.Origin));
    }
}
