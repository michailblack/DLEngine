#pragma once
#include "DLEngine/Core/Events/Event.h"

class MouseButtonEvent : public Event
{
public:
    uint8_t GetButton() const { return m_Button; }

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

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
    MouseButtonReleasedEvent(uint8_t button)
        : MouseButtonEvent(button)
    {}

    EVENT_CLASS_TYPE(MouseButtonReleased)
};

class MouseMovedEvent : public Event
{
public:
    MouseMovedEvent(int32_t x, int32_t y)
        : m_MouseX(x), m_MouseY(y)
    {}

    int32_t GetX() const { return m_MouseX; }
    int32_t GetY() const { return m_MouseY; }

    EVENT_CLASS_TYPE(MouseMoved)

private:
    int32_t m_MouseX, m_MouseY;
};
