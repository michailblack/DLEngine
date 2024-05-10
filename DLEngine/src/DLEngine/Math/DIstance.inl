#pragma once

namespace DLEngine
{
    namespace Math
    {
        inline float Distance(const Vec3& point, const Plane& plane)
        {
            return Dot(plane.Normal, point - plane.Origin);
        }
    }
}
