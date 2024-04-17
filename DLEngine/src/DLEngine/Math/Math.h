#pragma once
#include <DirectXMath.h>

namespace Math
{
    inline float ToRadians(float degrees)
    {
        return DirectX::XMConvertToRadians(degrees);
    }

    inline float ToDegrees(float radians)
    {
        return DirectX::XMConvertToDegrees(radians);
    }

    inline float Infinity()
    {
        return std::numeric_limits<float>::infinity();
    }
}
