﻿#pragma once
#include "DLEngine/Core/Events/Event.h"

namespace DLEngine
{
    class KeyEvent : public Event
    {
    public:
        uint8_t GetKeyCode() const noexcept { return m_KeyCode; }

        EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

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

        EVENT_CLASS_TYPE(EventType::KeyPressed)
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(uint8_t keyCode)
            : KeyEvent(keyCode)
        {}

        EVENT_CLASS_TYPE(EventType::KeyReleased)
    };
}
