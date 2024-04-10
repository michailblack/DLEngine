#pragma once
#include <functional>

enum class EventType
{
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMove,
    AppTick, AppUpdate, AppRender,
    KeyPressed, KeyReleased,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
    EventType GetEventType() const override { return GetStaticType(); }

class Event
{
public:
    Event() = default;

    Event(const Event&) = delete;
    Event(Event&&) = delete;
    Event& operator=(const Event&) = delete;
    Event& operator=(Event&&) = delete;

    bool Handled { false };

    virtual EventType GetEventType() const = 0;
};


class EventDispatcher
{
    template <typename T>
    using EventFn = std::function<bool(T&)>;

public:
    explicit EventDispatcher(Event& event)
        : m_Event(event)
    {}

    template <typename T>
    bool Dispatch(EventFn<T> callback)
    {
        if (m_Event.GetEventType() == T::GetStaticType())
        {
            m_Event.Handled |= callback(static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};
