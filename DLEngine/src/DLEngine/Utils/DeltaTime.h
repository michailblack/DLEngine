#pragma once

class DeltaTime
{
public:
    DeltaTime(float time = 0.0f)
        : m_Time(time)
    {}

    operator float() const noexcept { return m_Time; }

    float GetMilliseconds() const noexcept { return m_Time; }
    float GetSeconds() const noexcept { return m_Time * 1e-3f; }

private:
    float m_Time;
};