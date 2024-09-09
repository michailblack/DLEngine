#pragma once
#include "DLEngine/Core/Base.h"

#include <functional>

namespace DLEngine
{
    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMove,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory : uint8_t
    {
        None = 0,
        EventCategoryApplication = BIT(1),
        EventCategoryInput       = BIT(2),
        EventCategoryKeyboard    = BIT(3),
        EventCategoryMouse       = BIT(4),
        EventCategoryMouseButton = BIT(5)
    };

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return type; }\
    EventType GetEventType() const noexcept override { return GetStaticType(); }

#define EVENT_CLASS_CATEGORY(category) virtual uint8_t GetCategoryFlags() const noexcept override { return category; }

    class Event
    {
    public:
        Event() = default;

        Event(const Event&) = delete;
        Event(Event&&) = delete;
        Event& operator=(const Event&) = delete;
        Event& operator=(Event&&) = delete;

        bool Handled{ false };

        virtual EventType GetEventType() const noexcept = 0;
        virtual uint8_t GetCategoryFlags() const noexcept = 0;

        [[nodiscard]] bool IsInCategory(EventCategory category) const noexcept
        {
            return GetCategoryFlags() & category;
        }
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
}
