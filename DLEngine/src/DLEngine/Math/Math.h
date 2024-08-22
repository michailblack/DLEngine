#pragma once
#include <numbers>

namespace DLEngine::Math
{
    class Mat4x4;
    class Vec3;
    struct AABB;
    struct Ray;

    float ToRadians(float degrees) noexcept;
    float ToDegrees(float radians) noexcept;

    float Max(float a, float b);
    float Min(float a, float b);

    float Sign(float x);

    float Sqrt(float x) noexcept;

    float Pow(float base, float exponent) noexcept;
    float Exp(float x) noexcept;

    float Log(float x) noexcept;
    float Log2(float x) noexcept;
    float Log10(float x) noexcept;

    float Clamp(float value, float min, float max);

    float Cos(float angle) noexcept;
    float Sin(float angle) noexcept;

    float Acos(float x) noexcept;
    float AcosEst(float x) noexcept;

    Vec3 DirectionToSpace(const Vec3& direction, const Mat4x4& spaceTransformation) noexcept;
    Vec3 PointToSpace(const Vec3& point, const Mat4x4& spaceTransformation) noexcept;
    Ray RayToSpace(const Ray& ray, const Mat4x4& spaceTransformation) noexcept;
    AABB AABBToSpace(const AABB& aabb, const Mat4x4& spaceTransformation) noexcept;

    void BranchlessONB(const Vec3& n, Vec3& b1, Vec3& b2) noexcept;
    
    std::vector<Vec3> GenerateFibonacciHemispherePoints(uint32_t numPoints);

    struct Numeric
    {
        static constexpr float Inf{ std::numeric_limits<float>::infinity() };
        static constexpr float Max{ std::numeric_limits<float>::max() };
        static constexpr float Min{ std::numeric_limits<float>::min() };
        static constexpr float Pi { std::numbers::pi_v<float> };
    };
}
