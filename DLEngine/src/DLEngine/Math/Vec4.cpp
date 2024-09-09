#include "dlpch.h"
#include "Vec4.h"

namespace DLEngine::Math
{
    bool Vec4::operator==(const Vec4& v) const noexcept
    {
        return DirectX::XMVector4Equal(static_cast<DirectX::XMVECTOR>(*this), static_cast<DirectX::XMVECTOR>(v));
    }

    void Vec4::operator*=(float s) noexcept
    {
        *this = *this * s;
    }

    void Vec4::operator/=(float s) noexcept
    {
        *this = *this / s;
    }

    float Length(const Vec4& v) noexcept
    {
        return DirectX::XMVectorGetX(DirectX::XMVector4Length(static_cast<DirectX::XMVECTOR>(v)));
    }

    float Dot(const Vec4& v1, const Vec4& v2) noexcept
    {
        return DirectX::XMVectorGetX(DirectX::XMVector4Dot(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec4 Normalize(const Vec4& v) noexcept
    {
        return static_cast<Vec4>(DirectX::XMVector4Normalize(static_cast<DirectX::XMVECTOR>(v)));
    }

    Vec4 operator+(const Vec4& v1, const Vec4& v2) noexcept
    {
        return static_cast<Vec4>(DirectX::XMVectorAdd(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec4 operator-(const Vec4& v1, const Vec4& v2) noexcept
    {
        return static_cast<Vec4>(DirectX::XMVectorSubtract(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec4 operator*(const Vec4& v, float s) noexcept
    {
        return static_cast<Vec4>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
    }

    Vec4 operator*(float s, const Vec4& v) noexcept
    {
        return static_cast<Vec4>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
    }

    Vec4 operator/(const Vec4& v, float s) noexcept
    {
        return static_cast<Vec4>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), 1.0f / s));
    }
}