#pragma once
#include <DirectXMath.h>

namespace Math
{
    class Vec3
        : public DirectX::XMFLOAT3
    {
    public:
        Vec3()
            : XMFLOAT3 { 0.0f, 0.0f, 0.0f }
        {}
        Vec3(float x, float y, float z)
            : XMFLOAT3 { x, y, z }
        {}
        Vec3(float v)
            : XMFLOAT3 { v, v, v }
        {}
        Vec3(const XMFLOAT3& v)
            : XMFLOAT3 { v }
        {}
        Vec3(const DirectX::XMVECTOR& v)
        {
            DirectX::XMStoreFloat3(this, v);
        }

        ~Vec3() = default;

        Vec3(const Vec3&) = default;
        Vec3(Vec3&&) = default;
        Vec3& operator=(const Vec3&) = default;
        Vec3& operator=(Vec3&&) = default;

        explicit operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat3(this); }
    };

    inline float Length(const Vec3& v);
    inline float Dot(const Vec3& v1, const Vec3& v2);
    inline Vec3 Cross(const Vec3& v1, const Vec3& v2);
    inline Vec3 Normalize(const Vec3& v);

    inline Vec3 operator+(const Vec3& v1, const Vec3& v2);
    inline Vec3 operator-(const Vec3& v1, const Vec3& v2);
    inline Vec3 operator*(const Vec3& v, float s);
    inline Vec3 operator*(float s, const Vec3& v);
    inline Vec3 operator/(const Vec3& v, float s);
}

#include "Vec3.inl"
