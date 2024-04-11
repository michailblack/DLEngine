#pragma once
#include "DLEngine/Math/Vec2.h"

namespace Math
{
    inline float Length(const Vec2& v)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector2Length(static_cast<DirectX::XMVECTOR>(v)));
    }

    inline float Dot(const Vec2& v1, const Vec2& v2)
    {
        return DirectX::XMVectorGetX(DirectX::XMVector2Dot(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    inline Vec2 Cross(const Vec2& v1, const Vec2& v2)
    {
        return { DirectX::XMVector2Cross(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)) };
    }

    inline Vec2 Normalize(const Vec2& v)
    {
        return { DirectX::XMVector2Normalize(static_cast<DirectX::XMVECTOR>(v)) };
    }

    inline Vec2 operator+(const Vec2& v1, const Vec2& v2)
    {
        return { DirectX::XMVectorAdd(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)) };
    }

    inline Vec2 operator-(const Vec2& v1, const Vec2& v2)
    {
        return { DirectX::XMVectorSubtract(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)) };
    }

    inline Vec2 operator*(const Vec2& v, float s)
    {
        return { DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s) };
    }

    inline Vec2 operator*(float s, const Vec2& v)
    {
        return { DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s) };
    }

    inline Vec2 operator/(const Vec2& v, float s)
    {
        return { DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), 1.0f / s) };
    }

}
