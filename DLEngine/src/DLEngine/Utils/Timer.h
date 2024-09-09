#pragma once
#include <chrono>

namespace DLEngine
{
    class Timer
    {
    public:
        Timer(float frameTimeMS = 0.0) noexcept
            : m_FrameTimeMS(frameTimeMS)
        {
            Reset();
        }
        ~Timer() = default;

        Timer(const Timer&) = delete;
        Timer(Timer&&) = delete;
        Timer& operator=(const Timer&) = delete;
        Timer& operator=(Timer&&) = delete;

        void Reset() noexcept { m_Start = std::chrono::steady_clock::now(); }
        float ElapsedMS() const noexcept
        {
            return static_cast<float>(
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_Start).count()
            ) * 1.0e-3f;
        }

        bool FrameElapsed(float dtMS) const noexcept
        {
            if (dtMS >= m_FrameTimeMS)
                return true;
            return false;
        }

    private:
        std::chrono::time_point<std::chrono::steady_clock> m_Start;
        float m_FrameTimeMS;
    };
}
