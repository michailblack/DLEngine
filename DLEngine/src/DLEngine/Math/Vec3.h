#pragma once
#include <DirectXMath.h>

namespace DLEngine
{
    namespace Math
    {
        class Vec3
            : public DirectX::XMFLOAT3
        {
        public:
            constexpr Vec3() noexcept
                : XMFLOAT3{ 0.0f, 0.0f, 0.0f }
            {}
            constexpr explicit Vec3(float x, float y, float z) noexcept
                : XMFLOAT3{ x, y, z }
            {}
            constexpr explicit Vec3(float v) noexcept
                : XMFLOAT3{ v, v, v }
            {}
            constexpr explicit Vec3(const XMFLOAT3& v) noexcept
                : XMFLOAT3{ v }
            {}
            explicit Vec3(const DirectX::XMVECTOR& v) noexcept
            {
                DirectX::XMStoreFloat3(this, v);
            }

            constexpr ~Vec3() = default;

            constexpr Vec3(const Vec3&) = default;
            constexpr Vec3(Vec3&&) = default;
            constexpr Vec3& operator=(const Vec3&) = default;
            constexpr Vec3& operator=(Vec3&&) = default;

            explicit operator DirectX::XMVECTOR() const noexcept { return DirectX::XMLoadFloat3(this); }

            void operator+=(const Vec3& v) noexcept;
            void operator-=(const Vec3& v) noexcept;
            void operator*=(float s) noexcept;
            void operator/=(float s) noexcept;
        };

        inline float Length(const Vec3& v) noexcept;
        inline float Dot(const Vec3& v1, const Vec3& v2) noexcept;
        inline Vec3 Cross(const Vec3& v1, const Vec3& v2) noexcept;
        inline Vec3 Normalize(const Vec3& v) noexcept;
        inline Vec3 Exp(const Vec3& v) noexcept;
        inline Vec3 Pow(const Vec3& v1, const Vec3& v2) noexcept;
        inline Vec3 Min(const Vec3& v1, const Vec3& v2) noexcept;
        inline Vec3 Max(const Vec3& v1, const Vec3& v2) noexcept;

        inline bool operator==(const Vec3& v1, const Vec3& v2) noexcept;

        inline Vec3 operator+(const Vec3& v1, const Vec3& v2) noexcept;
        inline Vec3 operator-(const Vec3& v1, const Vec3& v2) noexcept;
        inline Vec3 operator-(const Vec3& v) noexcept;
        inline Vec3 operator*(const Vec3& v1, const Vec3& v2) noexcept;
        inline Vec3 operator*(const Vec3& v, float s) noexcept;
        inline Vec3 operator*(float s, const Vec3& v) noexcept;
        inline Vec3 operator/(const Vec3& v, float s) noexcept;
    }
}

#include "Vec3.inl"
