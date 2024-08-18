#pragma once
#include "DLEngine/Core/Events/Event.h"

namespace DLEngine
{
    class MouseButtonEvent : public Event
    {
    public:
        uint8_t GetButton() const noexcept { return m_Button; }

    protected:
        MouseButtonEvent(uint8_t button)
            : m_Button(button)
        {}

    private:
        uint8_t m_Button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(uint8_t button)
            : MouseButtonEvent(button)
        {}

        EVENT_CLASS_TYPE(EventType::MouseButtonPressed)
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(uint8_t button)
            : MouseButtonEvent(button)
        {}

        EVENT_CLASS_TYPE(EventType::MouseButtonReleased)
    };

    class MouseMovedEvent : public Event
    {
    public:
        MouseMovedEvent(int32_t x, int32_t y)
            : m_MouseX(x), m_MouseY(y)
        {}

        int32_t GetX() const noexcept { return m_MouseX; }
        int32_t GetY() const noexcept { return m_MouseY; }

        EVENT_CLASS_TYPE(EventType::MouseMoved)

    private:
        int32_t m_MouseX, m_MouseY;
    };

    class MouseScrolledEvent : public Event
    {
    public:
        MouseScrolledEvent(int32_t offset)
            : m_Offset(offset)
        {}

        int32_t GetOffset() const noexcept { return m_Offset; }

        EVENT_CLASS_TYPE(EventType::MouseScrolled)

    private:
        int32_t m_Offset;
    };
}
