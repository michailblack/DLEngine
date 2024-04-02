#pragma once
#include "Event.h"

enum class MouseButton
{
    Left = 0,
    Right,
    Middle
};

class MouseEvent : public Event
{
public:
    MouseEvent(uint64_t otherButtons, int16_t x, int16_t y)
        : m_OtherButtons(otherButtons), m_MouseX(x), m_MouseY(y)
    {}

    uint64_t GetOtherButtons() const { return m_OtherButtons; }
    int16_t GetX() const { return m_MouseX; }
    int16_t GetY() const { return m_MouseY; }

protected:
    uint64_t m_OtherButtons;
    int16_t m_MouseX, m_MouseY;
};

class MouseMovedEvent : public MouseEvent
{
public:
    MouseMovedEvent(uint64_t otherButtons, int16_t x, int16_t y)
        : MouseEvent(otherButtons, x, y)
    {}

    EVENT_CLASS_TYPE(MouseMoved)
};

class MouseButtonEvent : public MouseEvent
{
public:
    MouseButtonEvent(MouseButton button, uint64_t otherButtons, int16_t x, int16_t y)
        : MouseEvent(otherButtons, x, y)
        , m_Button(button)
    {}

    MouseButton GetButton() const { return m_Button; }

protected:
    MouseButton m_Button;
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
    MouseButtonPressedEvent(MouseButton button, uint64_t otherButtons, int16_t x, int16_t y)
        : MouseButtonEvent(button, otherButtons, x, y)
    {}

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
    MouseButtonReleasedEvent(MouseButton button, uint64_t otherButtons, int16_t x, int16_t y)
        : MouseButtonEvent(button, otherButtons, x, y)
    {}

    EVENT_CLASS_TYPE(MouseButtonReleased)
};
