#pragma once
#include "DLEngine/Core/Events/Event.h"

class KeyEvent : public Event
{
public:
    uint8_t GetKeyCode() const { return m_KeyCode; }

protected:
    KeyEvent(uint8_t keyCode)
        : m_KeyCode(keyCode)
    {}

private:
    uint8_t m_KeyCode;
};

class KeyPressedEvent : public KeyEvent
{
public:
    KeyPressedEvent(uint8_t keyCode)
        : KeyEvent(keyCode)
    {}

    EVENT_CLASS_TYPE(KeyPressed)
};

class KeyReleasedEvent : public KeyEvent
{
public:
    KeyReleasedEvent(uint8_t keyCode)
        : KeyEvent(keyCode)
    {}

    EVENT_CLASS_TYPE(KeyReleased)
};
