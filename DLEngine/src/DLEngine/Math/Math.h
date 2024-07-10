#pragma once

namespace DLEngine
{
    namespace Math
    {
        inline float ToRadians(float degrees) noexcept;
        inline float ToDegrees(float radians) noexcept;

        inline float Infinity() noexcept;
        inline float Pi() noexcept;

        inline float Max(float a, float b);
        inline float Min(float a, float b);

        inline float Sign(float x);

        inline float Pow(float base, float exponent) noexcept;
        inline float Exp(float x) noexcept;

        inline float Log10(float x) noexcept;

        inline float Clamp(float value, float min, float max);

        inline float Cos(float angle) noexcept;
        inline float Sin(float angle) noexcept;
    }
}

#include "Math.inl"
