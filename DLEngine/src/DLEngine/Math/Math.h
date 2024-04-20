#pragma once

namespace Math
{
    inline float ToRadians(float degrees);
    inline float ToDegrees(float radians);

    inline float Infinity();

    inline float Max(float a, float b);
    inline float Min(float a, float b);

    inline float Pow(float base, float exponent);

    inline float Clamp(float value, float min, float max);

    inline float Cos(float angle);
    inline float Sin(float angle);
}

#include "Math.inl"
