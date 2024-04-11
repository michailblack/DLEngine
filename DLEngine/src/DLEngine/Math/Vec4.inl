#pragma once
#include "DLEngine/Math/Vec4.h"

namespace Math
{
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
        return { DirectX::XMVector4Normalize(static_cast<DirectX::XMVECTOR>(v)) };
    }

    inline Vec4 operator+(const Vec4& v1, const Vec4& v2)
    {
        return { DirectX::XMVectorAdd(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)) };
    }

    inline Vec4 operator-(const Vec4& v1, const Vec4& v2)
    {
        return { DirectX::XMVectorSubtract(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)) };
    }

    inline Vec4 operator*(const Vec4& v, float s)
    {
        return { DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s) };
    }

    inline Vec4 operator*(float s, const Vec4& v)
    {
        return { DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s) };
    }

    inline Vec4 operator/(const Vec4& v, float s)
    {
        return { DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), 1.0f / s) };
    }
}
