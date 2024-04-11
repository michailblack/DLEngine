#pragma once
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
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            }
        {}
        Mat4x4(const Vec4& right, const Vec4 & up, const Vec4& forward, const Vec4& translation = Vec4 { 0.0f, 0.0f, 0.0f, 1.0f})
            : DirectX::XMFLOAT4X4 {
                right.x      , right.y      , right.z      , right.w,
                up.x         , up.y         , up.z         , up.w,
                forward.x    , forward.y    , forward.z    , forward.w,
                translation.x, translation.y, translation.z, translation.w
            }
        {}
        Mat4x4(const DirectX::XMFLOAT4X4& matrix)
            : DirectX::XMFLOAT4X4 { matrix }
        {}
        Mat4x4(const DirectX::XMMATRIX& matrix)
        {
            DirectX::XMStoreFloat4x4(this, matrix);
        }

        ~Mat4x4() = default;

        Mat4x4(const Mat4x4&) = default;
        Mat4x4(Mat4x4&&) = default;
        Mat4x4& operator=(const Mat4x4&) = default;
        Mat4x4& operator=(Mat4x4&&) = default;

        explicit operator DirectX::XMMATRIX() const { return DirectX::XMLoadFloat4x4(this); }
    };

    inline Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs);
    inline Vec4 operator*(const Vec4& v, const Mat4x4& m);
}

#include "Mat4x4.inl"
