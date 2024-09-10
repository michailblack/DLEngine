#pragma once
#include <DirectXMath.h>

namespace DLEngine::Math
{
    class Vec3;
    class Vec4;

    class Mat4x4 : public DirectX::XMFLOAT4X4
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
        explicit Mat4x4(const Vec3& row0, const Vec3& row1, const Vec3& row2, const Vec3& row3) noexcept;

        ~Mat4x4() = default;

        Mat4x4(const Mat4x4&) = default;
        Mat4x4(Mat4x4&&) = default;
        Mat4x4& operator=(const Mat4x4&) = default;
        Mat4x4& operator=(Mat4x4&&) = default;

        explicit operator DirectX::XMMATRIX() const noexcept { return DirectX::XMLoadFloat4x4(this); }

        static Mat4x4 Identity() noexcept;

        static Mat4x4 PerspectiveFov(float fovAngleY, float aspectRatio, float nearZ, float farZ) noexcept;
        static Mat4x4 Perspective(float viewWidth, float viewHeight, float nearZ, float farZ) noexcept;
        static Mat4x4 Orthographic(float viewWidth, float viewHeight, float nearZ, float farZ) noexcept;
        static Mat4x4 LookTo(const Vec3& position, const Vec3& forward, const Vec3& up) noexcept;

        static Mat4x4 Inverse(const Mat4x4& mat) noexcept;
        static Mat4x4 Transpose(const Mat4x4& mat) noexcept;

        static Mat4x4 Translate(const Vec3& translation) noexcept;
        static Mat4x4 Scale(const Vec3& scale) noexcept;
        static Mat4x4 Rotate(float pitch, float yaw, float roll) noexcept;
        static Mat4x4 Rotate(const Vec3& normalizedAxis, float angle) noexcept;

        static void Decompose(const Mat4x4& mat, Mat4x4& outScale, Mat4x4& outRotation, Mat4x4& outTranslation) noexcept;
    };

    Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs) noexcept;
    Vec4 operator*(const Vec4& v, const Mat4x4& m) noexcept;
}
