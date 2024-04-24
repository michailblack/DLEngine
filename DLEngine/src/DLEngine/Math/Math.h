#pragma once

namespace Math
{
    inline float ToRadians(float degrees) noexcept;
    inline float ToDegrees(float radians) noexcept;

    inline float Infinity() noexcept;

    inline float Max(float a, float b);
    inline float Min(float a, float b);

    inline float Pow(float base, float exponent) noexcept;

    inline float Clamp(float value, float min, float max);

    inline float Cos(float angle) noexcept;
    inline float Sin(float angle) noexcept;
}

#include "Math.inl"
