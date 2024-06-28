#pragma once
#include <DirectXMath.h>
#include <numbers>

namespace DLEngine::Math
{
    inline float ToRadians(float degrees) noexcept
    {
        return DirectX::XMConvertToRadians(degrees);
    }

    inline float ToDegrees(float radians) noexcept
    {
        return DirectX::XMConvertToDegrees(radians);
    }

    inline float Max(float a, float b)
    {
        return std::max(a, b);
    }

    inline float Min(float a, float b)
    {
        return std::min(a, b);
    }

    inline float Sign(float x)
    {
        return std::signbit(x) ? -1.0f : 1.0f;
    }

    inline float Pow(float base, float exponent) noexcept
    {
        return std::pow(base, exponent);
    }

    inline float Exp(float x) noexcept
    {
        return std::exp(x);
    }

    inline float Log10(float x) noexcept
    {
        return std::log10(x);
    }

    inline float Clamp(float value, float min, float max)
    {
        return std::clamp(value, min, max);
    }

    inline float Cos(float angle) noexcept
    {
        return DirectX::XMScalarCos(angle);
    }

    inline float Sin(float angle) noexcept
    {
        return DirectX::XMScalarSin(angle);
    }
}
