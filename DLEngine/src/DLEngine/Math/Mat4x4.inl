#pragma once
#include "DLEngine/Math/Mat4x4.h"

namespace Math
{
    inline Mat4x4 operator*(const Mat4x4& lhs, const Mat4x4& rhs)
    {
        return static_cast<Mat4x4>(DirectX::XMMatrixMultiply(static_cast<DirectX::XMMATRIX>(lhs), static_cast<DirectX::XMMATRIX>(rhs)));
    }

    inline Vec4 operator*(const Vec4& v, const Mat4x4& m)
    {
        return static_cast<Vec4>(DirectX::XMVector4Transform(static_cast<DirectX::XMVECTOR>(v), static_cast<DirectX::XMMATRIX>(m)));
    }
}
