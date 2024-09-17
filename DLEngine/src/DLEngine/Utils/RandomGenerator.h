#pragma once
#include <random>

namespace DLEngine
{
    class RandomGenerator
    {
    public:
        template <typename T>
        static T GenerateRandomInRange(T min, T max)
        {
            thread_local std::random_device rd;
            thread_local std::mt19937 eng(rd());

            if constexpr (std::is_integral_v<T>)
            {
                std::uniform_int_distribution<T> dist(min, max);
                return dist(eng);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                std::uniform_real_distribution<T> dist(min, max);
                return dist(eng);
            }
            else
            {
                return T{};
            }
        }

        template <typename T>
        static T GenerateRandom()
        {
            return GenerateRandomInRange<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        }

        static bool GenerateRandomBool(float probability = 0.5f)
        {
            return GenerateRandomInRange<float>(0.0f, 1.0f) <= probability;
        }
    };
}
