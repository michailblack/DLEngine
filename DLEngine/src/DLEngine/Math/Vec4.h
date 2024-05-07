#pragma once
#include <DirectXMath.h>

#include "DLEngine/Math/Vec3.h"

namespace DLEngine
{
    namespace Math
    {
        class Vec4
            : public DirectX::XMFLOAT4
        {
        public:
            constexpr Vec4() noexcept
                : XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f }
            {}
            constexpr explicit Vec4(float x, float y, float z, float w = 0.0f) noexcept
                : XMFLOAT4{ x, y, z, w }
            {}
            constexpr explicit Vec4(float v) noexcept
                : XMFLOAT4{ v, v, v, v }
            {}
            constexpr explicit Vec4(const Vec3& v, float w = 0.0f) noexcept
                : XMFLOAT4{ v.x, v.y, v.z, w }
            {}
            constexpr explicit Vec4(const XMFLOAT4& v) noexcept
                : XMFLOAT4{ v }
            {}
            explicit Vec4(const DirectX::XMVECTOR& v) noexcept
            {
                DirectX::XMStoreFloat4(this, v);
            }

            constexpr ~Vec4() = default;

            constexpr Vec4(const Vec4&) = default;
            constexpr Vec4(Vec4&&) = default;
            constexpr Vec4& operator=(const Vec4&) = default;
            constexpr Vec4& operator=(Vec4&&) = default;

            explicit operator DirectX::XMVECTOR() const noexcept { return DirectX::XMLoadFloat4(this); }

            inline void operator*=(float s) noexcept;
            inline void operator/=(float s) noexcept;

            constexpr Vec3 xyz() const noexcept { return Vec3{ x, y, z }; }
            constexpr const float* data() const noexcept { return &x; }
        };

        inline float Length(const Vec4& v) noexcept;
        inline float Dot(const Vec4& v1, const Vec4& v2) noexcept;
        inline Vec4 Normalize(const Vec4& v) noexcept;

        inline Vec4 operator+(const Vec4& v1, const Vec4& v2) noexcept;
        inline Vec4 operator-(const Vec4& v1, const Vec4& v2) noexcept;
        inline Vec4 operator*(const Vec4& v, float s) noexcept;
        inline Vec4 operator*(float s, const Vec4& v) noexcept;
        inline Vec4 operator/(const Vec4& v, float s) noexcept;

        inline Vec3 RotateQuaternion(const Vec3& v, const Vec3& normalizedAxis, float angle) noexcept;
    }
}

#include "Vec4.inl"
