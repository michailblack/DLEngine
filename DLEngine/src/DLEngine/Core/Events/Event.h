#pragma once

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
    virtual EventType GetEventType() const override { return GetStaticType(); }\
    virtual const wchar_t* GetName() const override { return L#type; }

enum class EventType
{
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    AppTick, AppUpdate, AppRender,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

class Event
{
public:
    Event() = default;
    virtual ~Event() = default;

    virtual EventType GetEventType() const = 0;
    virtual const wchar_t* GetName() const = 0;
public:
    bool IsHandled { false };
};
