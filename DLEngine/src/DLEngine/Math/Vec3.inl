#pragma once
#include "DLEngine/Math/Vec3.h"

namespace Math
{
    inline float Length(const Vec3& v)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector3Length(static_cast<DirectX::XMVECTOR>(v)));
    }

    inline float Dot(const Vec3& v1, const Vec3& v2)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector3Dot(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec3 Cross(const Vec3& v1, const Vec3& v2)
    {
        return { DirectX::XMVector3Cross(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)) };
    }

    inline Vec3 Normalize(const Vec3& v)
    {
        return { DirectX::XMVector3Normalize(static_cast<DirectX::XMVECTOR>(v)) };
    }

    inline Vec3 operator+(const Vec3& v1, const Vec3& v2)
    {
        return { DirectX::XMVectorAdd(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)) };
    }

    inline Vec3 operator-(const Vec3& v1, const Vec3& v2)
    {
        return { DirectX::XMVectorSubtract(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)) };
    }

    inline Vec3 operator*(const Vec3& v, float s)
    {
        return { DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s) };
    }

    inline Vec3 operator*(float s, const Vec3& v)
    {
        return { DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s) };
    }

    inline Vec3 operator/(const Vec3& v, float s)
    {
        return { DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), 1.0f / s) };
    }
}
