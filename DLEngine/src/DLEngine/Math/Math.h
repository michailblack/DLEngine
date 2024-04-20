#pragma once
#include <algorithm>
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

    inline float Max(float a, float b)
    {
        return std::max(a, b);
    }

    inline float Min(float a, float b)
    {
        return std::min(a, b);
    }

    inline float Pow(float base, float exponent)
    {
        return std::pow(base, exponent);
    }

    inline float Clamp(float value, float min, float max)
    {
        return std::clamp(value, min, max);
    }
}
