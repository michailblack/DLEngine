#pragma once
#include <chrono>

class Timer
{
public:
    Timer()
    {
        Reset();
    }
    ~Timer() = default;

    Timer(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete;

    void Reset()
    {
        m_Start = std::chrono::steady_clock::now();
    }

    double GetElapsedSeconds() const
    {
        return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - m_Start).count()) * 1.e-10;
    }
    double GetElapsedMilliseconds() const
    {
        return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - m_Start).count()) * 1.e-6;
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> m_Start;
};
