#pragma once
#include <DirectXMath.h>

namespace DLEngine
{
    namespace Math
    {
        class Vec2
            : public DirectX::XMFLOAT2
        {
        public:
            constexpr Vec2() noexcept
                : XMFLOAT2{ 0.0f, 0.0f }
            {}
            constexpr explicit Vec2(float x, float y) noexcept
                : XMFLOAT2{ x, y }
            {}
            constexpr explicit Vec2(float v) noexcept
                : XMFLOAT2{ v, v }
            {}
            constexpr explicit Vec2(const XMFLOAT2& v) noexcept
                : XMFLOAT2{ v }
            {}
            explicit Vec2(const DirectX::XMVECTOR& v) noexcept
            {
                XMStoreFloat2(this, v);
            }

            constexpr ~Vec2() = default;

            constexpr Vec2(const Vec2&) = default;
            constexpr Vec2(Vec2&&) = default;
            constexpr Vec2& operator=(const Vec2&) = default;
            constexpr Vec2& operator=(Vec2&&) = default;

            explicit operator DirectX::XMVECTOR() const noexcept { return DirectX::XMLoadFloat2(this); }

            void operator+=(const Vec2& v) noexcept;
            void operator-=(const Vec2& v) noexcept;
            void operator*=(float s) noexcept;
            void operator/=(float s) noexcept;
        };

        inline float Length(const Vec2& v) noexcept;
        inline float Dot(const Vec2& v1, const Vec2& v2) noexcept;
        inline Vec2 Cross(const Vec2& v1, const Vec2& v2) noexcept;
        inline Vec2 Normalize(const Vec2& v) noexcept;

        inline Vec2 operator+(const Vec2& v1, const Vec2& v2) noexcept;
        inline Vec2 operator-(const Vec2& v1, const Vec2& v2) noexcept;
        inline Vec2 operator*(const Vec2& v, float s) noexcept;
        inline Vec2 operator*(float s, const Vec2& v) noexcept;
        inline Vec2 operator/(const Vec2& v, float s) noexcept;
    }
}

#include "Vec2.inl"
