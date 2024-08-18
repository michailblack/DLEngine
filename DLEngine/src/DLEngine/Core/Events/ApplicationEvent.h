#pragma once
#include <DLEngine/Core/Events/Event.h>

namespace DLEngine
{
    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(uint32_t width, uint32_t height)
            : m_Width(width), m_Height(height)
        {}

        uint32_t GetWidth() const noexcept { return m_Width; }
        uint32_t GetHeight() const noexcept { return m_Height; }

        EVENT_CLASS_TYPE(EventType::WindowResize)

    private:
        uint32_t m_Width, m_Height;
    };

    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(EventType::WindowClose)
    };

    class AppTickEvent : public Event
    {
    public:
        AppTickEvent() = default;

        EVENT_CLASS_TYPE(EventType::AppTick)
    };

    class AppUpdateEvent : public Event
    {
    public:
        AppUpdateEvent() = default;

        EVENT_CLASS_TYPE(EventType::AppUpdate)
    };

    class AppRenderEvent : public Event
    {
    public:
        AppRenderEvent() = default;

        EVENT_CLASS_TYPE(EventType::AppRender)
    };
}
