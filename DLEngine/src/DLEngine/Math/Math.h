#pragma once
#include <numbers>

namespace DLEngine::Math
{
    class Vec3;

    inline float ToRadians(float degrees) noexcept;
    inline float ToDegrees(float radians) noexcept;

    inline float Max(float a, float b);
    inline float Min(float a, float b);

    inline float Sign(float x);

    inline float Pow(float base, float exponent) noexcept;
    inline float Exp(float x) noexcept;

    inline float Log(float x) noexcept;
    inline float Log2(float x) noexcept;
    inline float Log10(float x) noexcept;

    inline float Clamp(float value, float min, float max);

    inline float Cos(float angle) noexcept;
    inline float Sin(float angle) noexcept;
    
    inline std::vector<Vec3> GenerateFibonacciHemispherePoints(uint32_t numPoints);

    struct Numeric
    {
        static constexpr float Inf{ std::numeric_limits<float>::infinity() };
        static constexpr float Max{ std::numeric_limits<float>::max() };
        static constexpr float Min{ std::numeric_limits<float>::min() };
        static constexpr float Pi { std::numbers::pi_v<float> };
    };
}

#include "Math.inl"
