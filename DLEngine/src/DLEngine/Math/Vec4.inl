#pragma once
#include "DLEngine/Math/Vec4.h"

namespace Math
{
    inline void Vec4::operator*=(float s)
    {
        *this = *this * s;
    }

    inline void Vec4::operator/=(float s)
    {
        *this = *this / s;
    }


    inline float Length(const Vec4& v)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector4Length(static_cast<DirectX::XMVECTOR>(v)));
    }

    inline float Dot(const Vec4& v1, const Vec4& v2)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector4Dot(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec4 Normalize(const Vec4& v)
    {
        return static_cast<Vec4>(DirectX::XMVector4Normalize(static_cast<DirectX::XMVECTOR>(v)));
    }

    inline Vec4 operator+(const Vec4& v1, const Vec4& v2)
    {
        return static_cast<Vec4>(DirectX::XMVectorAdd(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec4 operator-(const Vec4& v1, const Vec4& v2)
    {
        return static_cast<Vec4>(DirectX::XMVectorSubtract(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec4 operator*(const Vec4& v, float s)
    {
        return static_cast<Vec4>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
    }

    inline Vec4 operator*(float s, const Vec4& v)
    {
        return static_cast<Vec4>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
    }

    inline Vec4 operator/(const Vec4& v, float s)
    {
        return static_cast<Vec4>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), 1.0f / s));
    }

    inline Vec3 RotateQuaternion(const Vec3& v, const Vec3& normalizedAxis, float angle)
    {
        const Vec4 quat = static_cast<Vec4>(DirectX::XMQuaternionRotationNormal(static_cast<DirectX::XMVECTOR>(normalizedAxis), angle));
        const Vec4 invQuat = static_cast<Vec4>(DirectX::XMQuaternionInverse(static_cast<DirectX::XMVECTOR>(quat)));

        Vec4 res = static_cast<Vec4>(DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(static_cast<DirectX::XMVECTOR>(quat), static_cast<DirectX::XMVECTOR>(Vec4 { v.x, v.y, v.z, 0.0f })), static_cast<DirectX::XMVECTOR>(invQuat)));

        return Vec3 { res.x, res.y, res.z };
    }
}
