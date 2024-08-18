#include "dlpch.h"
#include "Math.h"

#include "DLEngine/Math/Primitives.h"
#include "DLEngine/Math/Vec3.h"
#include "DLEngine/Math/Vec4.h"

namespace DLEngine::Math
{
    float ToRadians(float degrees) noexcept
    {
        return DirectX::XMConvertToRadians(degrees);
    }

    float ToDegrees(float radians) noexcept
    {
        return DirectX::XMConvertToDegrees(radians);
    }

    float Max(float a, float b)
    {
        return std::max(a, b);
    }

    float Min(float a, float b)
    {
        return std::min(a, b);
    }

    float Sign(float x)
    {
        return std::signbit(x) ? -1.0f : 1.0f;
    }

    float Sqrt(float x) noexcept
    {
        return std::sqrtf(x);
    }

    float Pow(float base, float exponent) noexcept
    {
        return std::pow(base, exponent);
    }

    float Exp(float x) noexcept
    {
        return std::exp(x);
    }

    float Log(float x) noexcept
    {
        return std::log(x);
    }

    float Log2(float x) noexcept
    {
        return std::log2(x);
    }

    float Log10(float x) noexcept
    {
        return std::log10(x);
    }

    float Clamp(float value, float min, float max)
    {
        if (value < min)
            return min;

        if (value > max)
            return max;

        return value;
    }

    float Cos(float angle) noexcept
    {
        return DirectX::XMScalarCos(angle);
    }

    float Sin(float angle) noexcept
    {
        return DirectX::XMScalarSin(angle);
    }

    std::vector<Vec3> GenerateFibonacciHemispherePoints(uint32_t numPoints)
    {
        std::vector<Vec3> points{};
        points.reserve(numPoints);

        static const float GOLDEN_RATIO{ (1.0f + std::sqrt(5.0f)) / 2.0f };

        for (uint32_t i{ 0u }; i < numPoints; ++i)
        {
            const float theta{ 2.0f * Numeric::Pi * i / GOLDEN_RATIO };
            const float phiCos{ 1.0f - (i + 0.5f) / numPoints };
            const float phiSin{ std::sqrt(1.0f - phiCos * phiCos) };
            const float thetaCos{ Cos(theta) };
            const float thetaSin{ Sin(theta) };

            points.emplace_back(Vec3{ phiSin * thetaCos, phiSin * thetaSin, phiCos });
        }

        return points;
    }

    Vec3 DirectionToSpace(const Vec3& direction, const Mat4x4& spaceTransformation) noexcept
    {
        const Vec4& transformedDirection{ Vec4{ direction.x, direction.y, direction.z, 0.0f } * spaceTransformation };
        return Vec3{ transformedDirection.x, transformedDirection.y, transformedDirection.z };
    }

    Vec3 PointToSpace(const Vec3& point, const Mat4x4& spaceTransformation) noexcept
    {
        const Vec4& transformedPoint{ Vec4{ point.x, point.y, point.z, 1.0f } * spaceTransformation };
        return Vec3{ transformedPoint.x, transformedPoint.y, transformedPoint.z };
    }

    Ray RayToSpace(const Ray& ray, const Mat4x4& spaceTransformation) noexcept
    {
        return Ray{ PointToSpace(ray.Origin, spaceTransformation), Normalize(DirectionToSpace(ray.Direction, spaceTransformation)) };
    }

    AABB AABBToSpace(const AABB& aabb, const Mat4x4& spaceTransformation) noexcept
    {
        const Vec3& minTransformed{ PointToSpace(aabb.Min, spaceTransformation) };
        const Vec3& maxTransformed{ PointToSpace(aabb.Max, spaceTransformation) };

        AABB result{};
        result.Min = Vec3{ Min(minTransformed.x, maxTransformed.x), Min(minTransformed.y, maxTransformed.y), Min(minTransformed.z, maxTransformed.z) };
        result.Max = Vec3{ Max(minTransformed.x, maxTransformed.x), Max(minTransformed.y, maxTransformed.y), Max(minTransformed.z, maxTransformed.z) };
        
        return result;
    }

}