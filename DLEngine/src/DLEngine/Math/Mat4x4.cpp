#include "dlpch.h"
#include "Mat4x4.h"

#include "DLEngine/Math/Vec3.h"
#include "DLEngine/Math/Vec4.h"

namespace DLEngine::Math
{
    Mat4x4::Mat4x4(const Vec3& row0, const Vec3& row1, const Vec3& row2, const Vec3& row3) noexcept
        : DirectX::XMFLOAT4X4{
        row0.x, row0.y, row0.z, 0.0f,
        row1.x, row1.y, row1.z, 0.0f,
        row2.x, row2.y, row2.z, 0.0f,
        row3.x, row3.y, row3.z, 1.0f
    }
    {
    }

    Mat4x4 Mat4x4::Identity() noexcept
    {
        Mat4x4 mat{};
        mat._11 = 1.0f;
        mat._22 = 1.0f;
        mat._33 = 1.0f;
        mat._44 = 1.0f;

        return mat;
    }

    Mat4x4 Mat4x4::PerspectiveFov(float fovAngleY, float aspectRatio, float nearZ, float farZ) noexcept
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ));
    }

    Mat4x4 Mat4x4::Perspective(float viewWidth, float viewHeight, float nearZ, float farZ) noexcept
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixPerspectiveLH(viewWidth, viewHeight, nearZ, farZ));
    }

    Mat4x4 Mat4x4::Orthographic(float viewWidth, float viewHeight, float nearZ, float farZ) noexcept
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ));
    }

    Mat4x4 Mat4x4::LookTo(const Vec3& position, const Vec3& forward, const Vec3& up) noexcept
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixLookToLH(static_cast<DirectX::XMVECTOR>(position), static_cast<DirectX::XMVECTOR>(forward), static_cast<DirectX::XMVECTOR>(up)));
    }

    Mat4x4 Mat4x4::Inverse(const Mat4x4& mat) noexcept
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixInverse(nullptr, static_cast<DirectX::XMMATRIX>(mat)));
    }

    Mat4x4 Mat4x4::Transpose(const Mat4x4& mat) noexcept
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixTranspose(static_cast<DirectX::XMMATRIX>(mat)));
    }

    Mat4x4 Mat4x4::Translate(const Vec3& translation) noexcept
    {
        Mat4x4 mat{ Identity() };

        mat._41 = translation.x;
        mat._42 = translation.y;
        mat._43 = translation.z;

        return mat;
    }

    Mat4x4 Mat4x4::Scale(const Vec3& scale) noexcept
    {
        Mat4x4 mat{ Identity() };

        mat._11 = scale.x;
        mat._22 = scale.y;
        mat._33 = scale.z;

        return mat;
    }

    Mat4x4 Mat4x4::Rotate(float pitch, float yaw, float roll) noexcept
    {
        Mat4x4 matRoll{ Identity() };
        Mat4x4 matPitch{ Identity() };
        Mat4x4 matYaw{ Identity() };

        matRoll._11 = std::cos(roll);
        matRoll._12 = -std::sin(roll);
        matRoll._21 = std::sin(roll);
        matRoll._22 = std::cos(roll);

        matPitch._22 = std::cos(pitch);
        matPitch._23 = -std::sin(pitch);
        matPitch._32 = std::sin(pitch);
        matPitch._33 = std::cos(pitch);

        matYaw._11 = std::cos(yaw);
        matYaw._13 = -std::sin(yaw);
        matYaw._31 = std::sin(yaw);
        matYaw._33 = std::cos(yaw);

        return matRoll * matPitch * matYaw;
    }

    Mat4x4 Mat4x4::Rotate(const Vec3& normalizedAxis, float angle) noexcept
    {
        const Vec4 quat = static_cast<Vec4>(DirectX::XMQuaternionRotationNormal(static_cast<DirectX::XMVECTOR>(normalizedAxis), angle));
        return static_cast<Mat4x4>(DirectX::XMMatrixRotationQuaternion(static_cast<DirectX::XMVECTOR>(quat)));
    }

    void Mat4x4::Decompose(const Mat4x4& mat, Mat4x4& outScale, Mat4x4& outRotation, Mat4x4& outTranslation) noexcept
    {
        const Vec3 translation{ mat._41, mat._42, mat._43 };
        outTranslation = Translate(translation);
        
        const Vec3 scale{
            Length(Vec3{ mat._11, mat._12, mat._13 }),
            Length(Vec3{ mat._21, mat._22, mat._23 }),
            Length(Vec3{ mat._31, mat._32, mat._33 })
        };
        outScale = Scale(scale);

        const Vec3 row0{ mat._11 / scale.x, mat._12 / scale.x, mat._13 / scale.x };
        const Vec3 row1{ mat._21 / scale.y, mat._22 / scale.y, mat._23 / scale.y };
        const Vec3 row2{ mat._31 / scale.z, mat._32 / scale.z, mat._33 / scale.z };
        outRotation = Mat4x4{ row0, row1, row2, Vec3{ 0.0f, 0.0f, 0.0f } };
    }

    Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs) noexcept
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixMultiply(static_cast<DirectX::XMMATRIX>(lhs), static_cast<DirectX::XMMATRIX>(rhs)));
    }

    Vec4 operator*(const Vec4& v, const Mat4x4& m) noexcept
    {
        return static_cast<Vec4>(DirectX::XMVector4Transform(static_cast<DirectX::XMVECTOR>(v), static_cast<DirectX::XMMATRIX>(m)));
    }
}