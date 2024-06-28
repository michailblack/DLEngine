#pragma once
#include <numbers>

namespace DLEngine::Math
{
    inline float ToRadians(float degrees) noexcept;
    inline float ToDegrees(float radians) noexcept;

    inline float Max(float a, float b);
    inline float Min(float a, float b);

    inline float Sign(float x);

    inline float Pow(float base, float exponent) noexcept;
    inline float Exp(float x) noexcept;

    inline float Log10(float x) noexcept;

    inline float Clamp(float value, float min, float max);

    inline float Cos(float angle) noexcept;
    inline float Sin(float angle) noexcept;

    struct Numeric
    {
        static constexpr float Inf{ std::numeric_limits<float>::infinity() };
        static constexpr float Max{ std::numeric_limits<float>::max() };
        static constexpr float Min{ std::numeric_limits<float>::min() };
        static constexpr float Pi { std::numbers::pi_v<float> };
    };
}

#include "Math.inl"
