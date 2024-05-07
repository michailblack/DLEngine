#pragma once
#include <cmath>
#include <DirectXMath.h>

#include "DLEngine/Math/Vec4.h"

namespace DLEngine
{
    namespace Math
    {
        class Mat4x4
            : public DirectX::XMFLOAT4X4
        {
        public:
            constexpr Mat4x4() noexcept
                : DirectX::XMFLOAT4X4{
                    0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f
            }
            {}
            explicit constexpr Mat4x4(const DirectX::XMFLOAT4X4& matrix) noexcept
                : DirectX::XMFLOAT4X4{ matrix }
            {}
            explicit Mat4x4(const DirectX::XMMATRIX& matrix) noexcept
            {
                DirectX::XMStoreFloat4x4(this, matrix);
            }

            ~Mat4x4() = default;

            Mat4x4(const Mat4x4&) = default;
            Mat4x4(Mat4x4&&) = default;
            Mat4x4& operator=(const Mat4x4&) = default;
            Mat4x4& operator=(Mat4x4&&) = default;

            explicit operator DirectX::XMMATRIX() const noexcept { return DirectX::XMLoadFloat4x4(this); }

            static Mat4x4 Identity() noexcept;

            static Mat4x4 Perspective(float fov, float aspectRatio, float zNear, float zFar) noexcept;
            static Mat4x4 View(const Vec3& right, const Vec3& up, const Vec3& forward, const Vec3& position) noexcept;

            static Mat4x4 Inverse(const Mat4x4& mat) noexcept;

            static Mat4x4 Translate(const Vec3& translation) noexcept;
            static Mat4x4 Scale(const Vec3& scale) noexcept;
            static Mat4x4 Rotate(float pitch, float yaw, float roll) noexcept;
            static Mat4x4 Rotate(const Vec3& normalizedAxis, float angle) noexcept;
        };

        inline Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs) noexcept;
        inline Vec4 operator*(const Vec4& v, const Mat4x4& m) noexcept;
    }
}

#include "Mat4x4.inl"
