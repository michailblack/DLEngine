#pragma once
#include <chrono>

class Timer
{
public:
    Timer(double frameTimeMS = 0.0) noexcept
        : m_FrameTimeMS(frameTimeMS)
    {
        Reset();
    }
    ~Timer() = default;

    Timer(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete;

    void Reset() noexcept
    {
        m_Start = std::chrono::steady_clock::now();
    }

    double GetDeltaTimeS() const noexcept
    {
        return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - m_Start).count()) * 1.e-10;
    }
    double GetDeltaTimeMS() const noexcept
    {
        return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - m_Start).count()) * 1.e-6;
    }

    bool FrameElapsed(double dtMS) const noexcept
    {
        if (dtMS >= m_FrameTimeMS)
            return true;
        return false;
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> m_Start;
    double m_FrameTimeMS;
};
