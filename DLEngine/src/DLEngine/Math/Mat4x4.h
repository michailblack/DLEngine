#pragma once
#include <cmath>
#include <DirectXMath.h>

#include "DLEngine/Math/Vec4.h"

namespace Math
{
    class Mat4x4
        : public DirectX::XMFLOAT4X4
    {
    public:
        Mat4x4()
            : DirectX::XMFLOAT4X4 {
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f
            }
        {}
        explicit Mat4x4(const DirectX::XMFLOAT4X4& matrix)
            : DirectX::XMFLOAT4X4 { matrix }
        {}
        explicit Mat4x4(const DirectX::XMMATRIX& matrix)
        {
            DirectX::XMStoreFloat4x4(this, matrix);
        }

        ~Mat4x4() = default;

        Mat4x4(const Mat4x4&) = default;
        Mat4x4(Mat4x4&&) = default;
        Mat4x4& operator=(const Mat4x4&) = default;
        Mat4x4& operator=(Mat4x4&&) = default;

        explicit operator DirectX::XMMATRIX() const { return DirectX::XMLoadFloat4x4(this); }

        static Mat4x4 Identity()
        {
            Mat4x4 mat {};
            mat._11 = 1.0f;
            mat._22 = 1.0f;
            mat._33 = 1.0f;
            mat._44 = 1.0f;

            return mat;
        }

        static Mat4x4 Perspective(float fov, float aspectRatio, float zNear, float zFar)
        {
            Mat4x4 mat {};
            mat._11 = (1.0f / std::tan(fov / 2.0f)) / aspectRatio;
            mat._22 = 1.0f / std::tan(fov / 2.0f);
            mat._33 = zFar / (zFar - zNear);
            mat._34 = 1.0f;
            mat._43 = -zFar * zNear / (zFar - zNear);

            return mat;
        }

        static Mat4x4 View(const Vec3& right, const Vec3& up, const Vec3& forward, const Vec3& position)
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

        static Mat4x4 Inverse(const Mat4x4& mat)
        {
            return static_cast<Mat4x4>(DirectX::XMMatrixInverse(nullptr, static_cast<DirectX::XMMATRIX>(mat)));
        }
    };

    inline Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs);
    inline Vec4 operator*(const Vec4& v, const Mat4x4& m);
}

#include "Mat4x4.inl"
