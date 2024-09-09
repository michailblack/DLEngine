#pragma once
#include <DirectXMath.h>

namespace DLEngine::Math
{
    class Vec4 : public DirectX::XMFLOAT4
    {
    public:
        constexpr Vec4() noexcept
            : XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f }
        {}
        constexpr explicit Vec4(float x, float y, float z, float w = 0.0f) noexcept
            : XMFLOAT4{ x, y, z, w }
        {}
        constexpr explicit Vec4(float v) noexcept
            : XMFLOAT4{ v, v, v, v }
        {}
        constexpr explicit Vec4(const XMFLOAT4& v) noexcept
            : XMFLOAT4{ v }
        {}
        explicit Vec4(const DirectX::XMVECTOR& v) noexcept
        {
            DirectX::XMStoreFloat4(this, v);
        }

        constexpr ~Vec4() = default;

        constexpr Vec4(const Vec4&) = default;
        constexpr Vec4(Vec4&&) = default;
        constexpr Vec4& operator=(const Vec4&) = default;
        constexpr Vec4& operator=(Vec4&&) = default;

        explicit operator DirectX::XMVECTOR() const noexcept { return DirectX::XMLoadFloat4(this); }

        bool operator==(const Vec4& v) const noexcept;

        void operator*=(float s) noexcept;
        void operator/=(float s) noexcept;
    };

    float Length(const Vec4& v) noexcept;
    float Dot(const Vec4& v1, const Vec4& v2) noexcept;
    Vec4 Normalize(const Vec4& v) noexcept;

    Vec4 operator+(const Vec4& v1, const Vec4& v2) noexcept;
    Vec4 operator-(const Vec4& v1, const Vec4& v2) noexcept;
    Vec4 operator*(const Vec4& v, float s) noexcept;
    Vec4 operator*(float s, const Vec4& v) noexcept;
    Vec4 operator/(const Vec4& v, float s) noexcept;

}
