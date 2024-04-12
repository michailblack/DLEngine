#pragma once
#include <DirectXMath.h>

#include "DLEngine/Math/Vec3.h"

namespace Math
{
    class Vec4
        : public DirectX::XMFLOAT4
    {
    public:
        Vec4()
            : XMFLOAT4 { 0.0f, 0.0f, 0.0f, 0.0f }
        {}
        explicit Vec4(float x, float y, float z, float w = 0.0f)
            : XMFLOAT4 { x, y, z, w }
        {}
        explicit Vec4(float v)
            : XMFLOAT4 { v, v, v, v }
        {}
        explicit Vec4(const Vec3& v, float w = 0.0f)
            : XMFLOAT4 { v.x, v.y, v.z, w }
        {}
        explicit Vec4(const XMFLOAT4& v)
            : XMFLOAT4 { v }
        {}
        explicit Vec4(const DirectX::XMVECTOR& v)
        {
            DirectX::XMStoreFloat4(this, v);
        }

        ~Vec4() = default;

        Vec4(const Vec4&) = default;
        Vec4(Vec4&&) = default;
        Vec4& operator=(const Vec4&) = default;
        Vec4& operator=(Vec4&&) = default;

        explicit operator DirectX::XMVECTOR() const { return DirectX::XMLoadFloat4(this); }

        inline void operator*=(float s);
        inline void operator/=(float s);
    };

    inline float Length(const Vec4& v);
    inline float Dot(const Vec4& v1, const Vec4& v2);
    inline Vec4 Normalize(const Vec4& v);

    inline Vec4 operator+(const Vec4& v1, const Vec4& v2);
    inline Vec4 operator-(const Vec4& v1, const Vec4& v2);
    inline Vec4 operator*(const Vec4& v, float s);
    inline Vec4 operator*(float s, const Vec4& v);
    inline Vec4 operator/(const Vec4& v, float s);

    inline Vec3 RotateQuaternion(const Vec3& v, const Vec3& normalizedAxis, float angle);
}

#include "Vec4.inl"
