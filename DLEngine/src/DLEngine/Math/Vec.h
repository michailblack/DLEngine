#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <complex>
#include <numbers>
#include <numeric>

namespace Math
{
    template <uint8_t N, typename T>
    struct Vec
    {
        static_assert(std::is_arithmetic_v<T>);

        Vec() = default;
        Vec(const std::initializer_list<T>& vec)
        {
            assert(vec.size() == N && "Vec must have the correct number of elements");
            std::ranges::copy_n(vec.begin(), N, Data.begin());
        }

        Vec(const T& value)
            : Data({ value })
        {}

        std::array<T, N> Data;

        T& operator[](uint8_t index) { return Data[index]; }

        Vec operator+(const Vec& other) const
        {
            Vec result;
            std::ranges::transform(Data, other.Data, result.Data.begin(), std::plus<T>());
            return result;
        }
        Vec operator-(const Vec& other) const
        {
            Vec result;
            std::ranges::transform(Data, other.Data, result.Data.begin(), std::minus<T>());
            return result;
        }
        Vec operator*(const T& value) const
        {
            Vec result;
            std::ranges::transform(Data, result.Data.begin(), [value](const T& element) { return element * value; });
            return result;
        }
        Vec operator/(const T& value) const
        {
            Vec result;
            std::ranges::transform(Data, result.Data.begin(), [value](const T& element) { return element / value; });
            return result;
        }
        Vec operator-() const
        {
            Vec result;
            std::ranges::transform(Data, result.Data.begin(), [](const T& element) { return -element; });
            return result;
        }

        Vec operator+=(const Vec& other) { return *this = *this + other; }
        Vec operator-=(const Vec& other) { return *this = *this - other; }
        Vec operator*=(const T& value) { return *this = *this * value; }

        bool operator==(const Vec& other) const { return Data == other.Data; }
        bool operator!=(const Vec& other) const { return !(*this == other); }

        template <typename U>
        explicit operator Vec<N, U>() const
        {
            Vec<N, U> result;
            std::ranges::transform(Data, result.Data.begin(), [](const T& element) { return static_cast<U>(element); });
            return result;
        }
    };

    template <uint8_t N, typename T>
    T Dot(const Vec<N, T>& a, const Vec<N, T>& b)
    {
        return std::inner_product(a.Data.begin(), a.Data.end(), b.Data.begin(), T {0});
    }

    template <uint8_t N, typename T>
    T Length(const Vec<N, T>& a)
    {
        return std::sqrt(Dot(a, a));
    }

    template <uint8_t N, typename T>
    Vec<N, T> Normalize(const Vec<N, T>& a)
    {
        return a / Length(a);
    }

    template <uint8_t N, typename T>
    Vec<N, T> Reflect(const Vec<N, T>& direction, const Vec<N, T>& normal)
    {
        return direction - normal * 2.0f * Dot(direction, normal);
    }

    template <uint8_t N, typename T>
    T Angle(const Vec<N, T>& a, const Vec<N, T>& b)
    {
        return std::acos(Dot(a, b) / (Length(a) * Length(b)));
    }

    template <typename T>
    T RadiansToDegrees(T radians)
    {
        static_assert(std::is_arithmetic_v<T>);
        return radians * 180.0f / std::numbers::pi_v<T>;
    }

    template <typename T>
    T DegreesToRadians(T degrees)
    {
        static_assert(std::is_arithmetic_v<T>);
        return degrees * std::numbers::pi_v<T> / 180.0f;
    }

    template <typename T>
    using Vec2 = Vec<2, T>;
    using Vec2f = Vec2<float>;

    template <typename T>
    using Vec3 = Vec<3, T>;
    using Vec3f = Vec3<float>;
}
