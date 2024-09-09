#include "dlpch.h"
#include "Vec3.h"

#include "DLEngine/Math/Vec4.h"

namespace DLEngine::Math
{
    void Vec3::operator+=(const Vec3& v) noexcept
    {
        *this = *this + v;
    }

    void Vec3::operator-=(const Vec3& v) noexcept
    {
        *this = *this - v;
    }

    void Vec3::operator*=(float s) noexcept
    {
        *this = *this * s;
    }

    void Vec3::operator/=(float s) noexcept
    {
        *this = *this / s;
    }

    float Length(const Vec3& v) noexcept
    {
        return DirectX::XMVectorGetX(DirectX::XMVector3Length(static_cast<DirectX::XMVECTOR>(v)));
    }

    float Dot(const Vec3& v1, const Vec3& v2) noexcept
    {
        return DirectX::XMVectorGetX(DirectX::XMVector3Dot(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec3 Cross(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVector3Cross(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec3 Normalize(const Vec3& v) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVector3Normalize(static_cast<DirectX::XMVECTOR>(v)));
    }

    Vec3 Exp(const Vec3& v) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorExpE(static_cast<DirectX::XMVECTOR>(v)));
    }

    Vec3 Pow(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorPow(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec3 Min(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorMin(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec3 Max(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorMax(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    bool operator==(const Vec3& v1, const Vec3& v2) noexcept
    {
        return DirectX::XMVector3Equal(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2));
    }

    Vec3 operator+(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorAdd(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec3 operator-(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorSubtract(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec3 operator-(const Vec3& v) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorNegate(static_cast<DirectX::XMVECTOR>(v)));
    }

    Vec3 operator*(const Vec3& v1, const Vec3& v2) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorMultiply(static_cast<DirectX::XMVECTOR>(v1), static_cast<DirectX::XMVECTOR>(v2)));
    }

    Vec3 operator*(const Vec3& v, float s) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
    }

    Vec3 operator*(float s, const Vec3& v) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), s));
    }

    Vec3 operator/(const Vec3& v, float s) noexcept
    {
        return static_cast<Vec3>(DirectX::XMVectorScale(static_cast<DirectX::XMVECTOR>(v), 1.0f / s));
    }

    Vec3 RotateQuaternion(const Vec3& v, const Vec3& normalizedAxis, float angle) noexcept
    {
        const Vec4 quat{ static_cast<Vec4>(DirectX::XMQuaternionRotationNormal(static_cast<DirectX::XMVECTOR>(normalizedAxis), angle)) };
        const Vec4 invQuat{ static_cast<Vec4>(DirectX::XMQuaternionInverse(static_cast<DirectX::XMVECTOR>(quat))) };

        const Vec4 res{ static_cast<Vec4>(DirectX::XMQuaternionMultiply(
            DirectX::XMQuaternionMultiply(static_cast<DirectX::XMVECTOR>(quat),
            static_cast<DirectX::XMVECTOR>(Vec4{ v.x, v.y, v.z, 0.0f })),
            static_cast<DirectX::XMVECTOR>(invQuat)
        )) };

        return Vec3{ res.x, res.y, res.z };
    }
}