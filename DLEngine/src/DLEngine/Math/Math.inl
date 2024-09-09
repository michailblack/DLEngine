#pragma once
#include "DLEngine/Math/Vec3.h"

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

    float Log(float x) noexcept
    {
        return std::log(x);
    }

    float Log2(float x) noexcept
    {
        return std::log2(x);
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

    std::vector<Vec3> GenerateFibonacciHemispherePoints(uint32_t numPoints)
    {
        std::vector<Vec3> points{};
        points.reserve(numPoints);

        static const float GOLDEN_RATIO{ (1.0f + std::sqrt(5.0f)) / 2.0f };

        for (uint32_t i{ 0u }; i < numPoints; ++i)
        {
            const float theta{ 2.0f * Numeric::Pi * i / GOLDEN_RATIO };
            const float phiCos{ 1.0f - (i + 0.5f) / numPoints };
            const float phiSin{ std::sqrt(1.0f - phiCos * phiCos) };
            const float thetaCos{ Cos(theta) };
            const float thetaSin{ Sin(theta) };

            points.emplace_back(Vec3{ phiSin * thetaCos, phiSin * thetaSin, phiCos });
        }

        return points;
    }
}
