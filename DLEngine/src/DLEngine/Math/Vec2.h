#pragma once
#include <DirectXMath.h>

namespace Math
{
    class Vec2
        : public DirectX::XMFLOAT2
    {
    public:
        Vec2()
            : XMFLOAT2 { 0.0f, 0.0f }
        {}
        explicit Vec2(float x, float y)
            : XMFLOAT2 { x, y }
        {}
        explicit Vec2(float v)
            : XMFLOAT2 { v, v }
        {}
        explicit Vec2(const XMFLOAT2& v)
            : XMFLOAT2 { v }
        {}
        explicit Vec2(const DirectX::XMVECTOR& v)
        {
            XMStoreFloat2(this, v);
        }

        ~Vec2() = default;

        Vec2(const Vec2&) = default;
        Vec2(Vec2&&) = default;
        Vec2& operator=(const Vec2&) = default;
        Vec2& operator=(Vec2&&) = default;

        explicit operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat2(this); }

        void operator+=(const Vec2& v);
        void operator-=(const Vec2& v);
        void operator*=(float s);
        void operator/=(float s);
    };

    inline float Length(const Vec2& v);
    inline float Dot(const Vec2& v1, const Vec2& v2);
    inline Vec2 Cross(const Vec2& v1, const Vec2& v2);
    inline Vec2 Normalize(const Vec2& v);

    inline Vec2 operator+(const Vec2& v1, const Vec2& v2);
    inline Vec2 operator-(const Vec2& v1, const Vec2& v2);
    inline Vec2 operator*(const Vec2& v, float s);
    inline Vec2 operator*(float s, const Vec2& v);
    inline Vec2 operator/(const Vec2& v, float s);
}

#include "Vec2.inl"
