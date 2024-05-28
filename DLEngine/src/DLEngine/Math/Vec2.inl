#pragma once
#include "DLEngine/Math/Vec2.h"

namespace DLEngine
{
    namespace Math
    {
        inline void Vec2::operator+=(const Vec2& v) noexcept
        {
            *this = *this + v;
        }

        inline void Vec2::operator-=(const Vec2& v) noexcept
        {
            *this = *this - v;
        }

        inline void Vec2::operator*=(float s) noexcept
        {
            *this = *this * s;
        }

        inline void Vec2::operator/=(float s) noexcept
        {
            *this = *this / s;
        }

        inline float Length(const Vec2& v) noexcept
        {
            return DirectX::XMVectorGetX(DirectX::XMVector2Length(static_cast<DirectX::XMVECTOR>(v)));
        }

        inline float Dot(const Vec2& v1, const Vec2& v2) noexcept
        {
            return DirectX::XMVectorGetX(DirectX::XMVector2Dot(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
        }

        inline Vec2 Cross(const Vec2& v1, const Vec2& v2) noexcept
        {
            return static_cast<Vec2>(DirectX::XMVector2Cross(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
        }

        inline Vec2 Normalize(const Vec2& v) noexcept
        {
            return static_cast<Vec2>(DirectX::XMVector2Normalize(static_cast<DirectX::XMVECTOR>(v)));
        }

        inline Vec2 operator+(const Vec2& v1, const Vec2& v2) noexcept
        {
            return static_cast<Vec2>(DirectX::XMVectorAdd(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
        }

        inline Vec2 operator-(const Vec2& v1, const Vec2& v2) noexcept
        {
            return static_cast<Vec2>(DirectX::XMVectorSubtract(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
        }

        inline Vec2 operator*(const Vec2& v, float s) noexcept
        {
            return static_cast<Vec2>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
        }

        inline Vec2 operator*(float s, const Vec2& v) noexcept
        {
            return static_cast<Vec2>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
        }

        inline Vec2 operator/(const Vec2& v, float s) noexcept
        {
            return static_cast<Vec2>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), 1.0f / s));
        }

    }
}
