#pragma once
#include <DirectXMath.h>

namespace Math
{
    class Vec3
        : public DirectX::XMFLOAT3
    {
    public:
        constexpr Vec3()
            : XMFLOAT3 { 0.0f, 0.0f, 0.0f }
        {}
        constexpr explicit Vec3(float x, float y, float z)
            : XMFLOAT3 { x, y, z }
        {}
        constexpr explicit Vec3(float v)
            : XMFLOAT3 { v, v, v }
        {}
        constexpr explicit Vec3(const XMFLOAT3& v)
            : XMFLOAT3 { v }
        {}
        explicit Vec3(const DirectX::XMVECTOR& v)
        {
            DirectX::XMStoreFloat3(this, v);
        }

        constexpr ~Vec3() = default;

        constexpr Vec3(const Vec3&) = default;
        constexpr Vec3(Vec3&&) = default;
        constexpr Vec3& operator=(const Vec3&) = default;
        constexpr Vec3& operator=(Vec3&&) = default;

        explicit operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat3(this); }

        void operator+=(const Vec3& v);
        void operator-=(const Vec3& v);
        void operator*=(float s);
        void operator/=(float s);
    };

    inline float Length(const Vec3& v);
    inline float Dot(const Vec3& v1, const Vec3& v2);
    inline Vec3 Cross(const Vec3& v1, const Vec3& v2);
    inline Vec3 Normalize(const Vec3& v);
    inline Vec3 Exp(const Vec3& v);
    inline Vec3 Pow(const Vec3& v1, const Vec3& v2);
    inline Vec3 Min(const Vec3& v1, const Vec3& v2);
    inline Vec3 Max(const Vec3& v1, const Vec3& v2);

    inline bool operator==(const Vec3& v1, const Vec3& v2);

    inline Vec3 operator+(const Vec3& v1, const Vec3& v2);
    inline Vec3 operator-(const Vec3& v1, const Vec3& v2);
    inline Vec3 operator-(const Vec3& v);
    inline Vec3 operator*(const Vec3& v1, const Vec3& v2);
    inline Vec3 operator*(const Vec3& v, float s);
    inline Vec3 operator*(float s, const Vec3& v);
    inline Vec3 operator/(const Vec3& v, float s);
}

#include "Vec3.inl"
