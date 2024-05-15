#pragma once
#include "DLEngine/Math/Vec3.h"

namespace Math
{
    inline void Vec3::operator+=(const Vec3& v) noexcept
    {
        *this = *this + v;
    }

    inline void Vec3::operator-=(const Vec3& v) noexcept
    {
        *this = *this - v;
    }

    inline void Vec3::operator*=(float s) noexcept
    {
        *this = *this * s;
    }

    inline void Vec3::operator/=(float s) noexcept
    {
        *this = *this / s;
    }

    inline float Length(const Vec3& v) noexcept
    {
        return DirectX::XMVectorGetX(DirectX::XMVector3Length(static_cast<DirectX::XMVECTOR>(v)));
    }

    inline float Dot(const Vec3& v1, const Vec3& v2) noexcept
    {
        return DirectX::XMVectorGetX(DirectX::XMVector3Dot(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec3 Cross(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVector3Cross(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec3 Normalize(const Vec3& v) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVector3Normalize(static_cast<DirectX::XMVECTOR>(v)));
    }

    inline Vec3 Exp(const Vec3& v) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorExpE(static_cast<DirectX::XMVECTOR>(v)));
    }

    inline Vec3 Pow(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorPow(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec3 Min(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorMin(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec3 Max(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorMax(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline bool operator==(const Vec3& v1, const Vec3& v2) noexcept
    {
        return DirectX::XMVector3Equal(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2));
    }

    inline Vec3 operator+(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorAdd(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec3 operator-(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorSubtract(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec3 operator-(const Vec3& v) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorNegate(static_cast<DirectX::XMVECTOR>(v)));
    }

    inline Vec3 operator*(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorMultiply(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec3 operator*(const Vec3& v, float s) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
    }

    inline Vec3 operator*(float s, const Vec3& v) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
    }

    inline Vec3 operator/(const Vec3& v, float s) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), 1.0f / s));
    }
}
