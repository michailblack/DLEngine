#pragma once
#include <DirectXMath.h>
#include <numbers>

namespace DLEngine
{
    namespace Math
    {
        inline float ToRadians(float degrees) noexcept
        {
            return DirectX::XMConvertToRadians(degrees);
        }

        inline float ToDegrees(float radians) noexcept
        {
            return DirectX::XMConvertToDegrees(radians);
        }

        inline float Infinity() noexcept
        {
            return std::numeric_limits<float>::infinity();
        }

        inline float Pi() noexcept
        {
            return std::numbers::pi_v<float>;
        }

        inline float Max(float a, float b)
        {
            return std::max(a, b);
        }

        inline float Min(float a, float b)
        {
            return std::min(a, b);
        }

        inline float Pow(float base, float exponent) noexcept
        {
            return std::pow(base, exponent);
        }

        inline float Clamp(float value, float min, float max)
        {
            return std::clamp(value, min, max);
        }

        inline float Cos(float angle) noexcept
        {
            return std::cos(angle);
        }

        inline float Sin(float angle) noexcept
        {
            return std::sin(angle);
        }
    }
}
