#pragma once
#include "DLEngine/Math/Mat4x4.h"

namespace Math
{
    inline Mat4x4 Mat4x4::Identity()
    {
        Mat4x4 mat {};
        mat._11 = 1.0f;
        mat._22 = 1.0f;
        mat._33 = 1.0f;
        mat._44 = 1.0f;

        return mat;
    }

    inline Mat4x4 Mat4x4::Perspective(float fov, float aspectRatio, float zNear, float zFar)
    {
        Mat4x4 mat {};
        mat._11 = (1.0f / std::tan(fov / 2.0f)) / aspectRatio;
        mat._22 = 1.0f / std::tan(fov / 2.0f);
        mat._33 = zFar / (zFar - zNear);
        mat._34 = 1.0f;
        mat._43 = -zFar * zNear / (zFar - zNear);

        return mat;
    }

    inline Mat4x4 Mat4x4::View(const Vec3& right, const Vec3& up, const Vec3& forward, const Vec3& position)
    {
        Mat4x4 mat {};
        mat._11 = right.x;
        mat._12 = up.x;
        mat._13 = forward.x;
        mat._14 = 0.0f;

        mat._21 = right.y;
        mat._22 = up.y;
        mat._23 = forward.y;
        mat._24 = 0.0f;

        mat._31 = right.z;
        mat._32 = up.z;
        mat._33 = forward.z;
        mat._34 = 0.0f;

        mat._41 = -Dot(right, position);
        mat._42 = -Dot(up, position);
        mat._43 = -Dot(forward, position);
        mat._44 = 1.0f;

        return mat;
    }

    inline Mat4x4 Mat4x4::Inverse(const Mat4x4& mat)
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixInverse(nullptr, static_cast<DirectX::XMMATRIX>(mat)));
    }

    inline Mat4x4 Mat4x4::Translate(const Vec3& translation)
    {
        Mat4x4 mat { Identity() };

        mat._41 = translation.x;
        mat._42 = translation.y;
        mat._43 = translation.z;

        return mat;
    }

    inline Mat4x4 Mat4x4::Scale(const Vec3& scale)
    {
        Mat4x4 mat { Identity() };

        mat._11 = scale.x;
        mat._22 = scale.y;
        mat._33 = scale.z;

        return mat;
    }

    inline Mat4x4 Mat4x4::Rotate(float pitch, float yaw, float roll)
    {
        Mat4x4 matRoll  { Identity() };
        Mat4x4 matPitch { Identity() };
        Mat4x4 matYaw   { Identity() };

        matRoll._11 =  std::cos(roll);
        matRoll._12 = -std::sin(roll);
        matRoll._21 =  std::sin(roll);
        matRoll._22 =  std::cos(roll);

        matPitch._22 =  std::cos(pitch);
        matPitch._23 = -std::sin(pitch);
        matPitch._32 =  std::sin(pitch);
        matPitch._33 =  std::cos(pitch);

        matYaw._11 =  std::cos(yaw);
        matYaw._13 = -std::sin(yaw);
        matYaw._31 =  std::sin(yaw);
        matYaw._33 =  std::cos(yaw);

        return matRoll * matPitch * matYaw;
    }

    inline Mat4x4 Mat4x4::Rotate(const Vec3& normalizedAxis, float angle)
    {
        const Vec4 quat = static_cast<Vec4>(DirectX::XMQuaternionRotationNormal(static_cast<DirectX::XMVECTOR>(normalizedAxis), angle));
        return static_cast<Mat4x4>(DirectX::XMMatrixRotationQuaternion(static_cast<DirectX::XMVECTOR>(quat)));
    }

    inline Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs)
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixMultiply(static_cast<DirectX::XMMATRIX>(lhs), static_cast<DirectX::XMMATRIX>(rhs)));
    }

    inline Vec4 operator*(const Vec4& v, const Mat4x4& m)
    {
        return static_cast<Vec4>(DirectX::XMVector4Transform(static_cast<DirectX::XMVECTOR>(v), static_cast<DirectX::XMMATRIX>(m)));
    }
}
