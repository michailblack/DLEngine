#pragma once
#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Event.h"

class IEventHandlerWrapper
{
public:
    IEventHandlerWrapper() = default;
    virtual ~IEventHandlerWrapper() = default;

    IEventHandlerWrapper(const IEventHandlerWrapper&) = delete;
    IEventHandlerWrapper(IEventHandlerWrapper&&) = delete;
    IEventHandlerWrapper& operator=(const IEventHandlerWrapper&) = delete;
    IEventHandlerWrapper& operator=(IEventHandlerWrapper&&) = delete;

public:
    void HandleEvent(const Event& event) { InvokeHandler(event); }

    virtual std::string GetType() const = 0;

protected:
    virtual void InvokeHandler(const Event& event) = 0;
};

template <typename EventTypeEnum>
class EventHandlerWrapper
    : public IEventHandlerWrapper
{
    static_assert(std::is_base_of_v<Event, EventTypeEnum>, "EventTypeEnum must be derived from Event");

    using EventHandler = std::function<void(const EventTypeEnum&)>;
public:
    explicit EventHandlerWrapper(const EventHandler& handler)
        : m_Handler(handler)
    {}
    ~EventHandlerWrapper() override = default;

    EventHandlerWrapper(const EventHandlerWrapper&) = delete;
    EventHandlerWrapper(EventHandlerWrapper&&) = delete;
    EventHandlerWrapper& operator=(const EventHandlerWrapper&) = delete;
    EventHandlerWrapper& operator=(EventHandlerWrapper&&) = delete;

    std::string GetType() const override { return m_Handler.target_type().name(); }

protected:
    void InvokeHandler(const Event& event) override
    {
        if (event.GetEventType() == EventTypeEnum::GetStaticType())
        {
            m_Handler(static_cast<const EventTypeEnum&>(event));
        }
    }

private:
    EventHandler m_Handler;
};

class EventBus
{
    template <typename EventTypeEnum>
    using EventHandler = std::function<void(const EventTypeEnum&)>;

private:
    EventBus() = default;
    ~EventBus() = default;
public:
    EventBus(const EventBus&) = delete;
    EventBus(EventBus&&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus& operator=(EventBus&&) = delete;

    static EventBus& Get()
    {
        static EventBus s_Instance;
        return s_Instance;
    }

public:
    template <typename EventTypeEnum>
    void Subscribe(const EventHandler<EventTypeEnum>& callback)
    {
        std::unique_ptr<IEventHandlerWrapper> wrapper = std::make_unique<EventHandlerWrapper<EventTypeEnum>>(callback);
        const auto& subscribers = m_Subscribers[EventTypeEnum::GetStaticType()];
        const auto& it = std::ranges::find_if(subscribers, [&wrapper](const auto& subscriber) { return subscriber->GetType() == wrapper->GetType(); });
        if (it != subscribers.end())
        {
            assert(false && "Subscriber already exists");
            return;
        }

        m_Subscribers[EventTypeEnum::GetStaticType()].emplace_back(std::move(wrapper));
    }

    template <typename EventTypeEnum>
    void Unsubscribe(const EventHandler<EventTypeEnum>& callback)
    {
        std::unique_ptr<IEventHandlerWrapper> wrapper = std::make_unique<EventHandlerWrapper<EventTypeEnum>>(callback);
        const auto& subscribers = m_Subscribers[EventTypeEnum::GetStaticType()];
        const auto& it = std::ranges::find_if(subscribers, [&wrapper](const auto& subscriber) { return subscriber->GetType() == wrapper->GetType(); });
        if (it != subscribers.end())
        {
            m_Subscribers[EventTypeEnum::GetStaticType()].erase(it);
            return;
        }

        assert(false && "There is no such a subscriber");
    }

    template <typename EventTypeEnum>
    void TriggerEvent(const EventTypeEnum& event)
    {
        static_assert(std::is_base_of_v<Event, EventTypeEnum>, "EventTypeEnum must be derived from Event");

        const auto& subscribers = m_Subscribers[event.GetEventType()];
        std::ranges::for_each(subscribers, [&event](const auto& subscriber) { subscriber->HandleEvent(event); });
    }

    template <typename EventTypeEnum>
    void QueueEvent(const EventTypeEnum& event)
    {
        static_assert(std::is_base_of_v<Event, EventTypeEnum>, "EventTypeEnum must be derived from Event");

        m_Events.emplace_back(std::make_unique<EventTypeEnum>(event));
    }

    void DispatchEvents()
    {
        m_Events.erase(std::ranges::remove_if(m_Events, [this](const std::unique_ptr<Event>& event)
        {
            if (!event->IsHandled)
                TriggerEvent(*event);

            return true;
        }).begin(), m_Events.end());
    }

    void Shutdown()
    {
        m_Events.clear();
        m_Subscribers.clear();
    }

private:
    std::vector<std::unique_ptr<Event>> m_Events;
    std::unordered_map<EventType, std::vector<std::unique_ptr<IEventHandlerWrapper>>> m_Subscribers;
};
