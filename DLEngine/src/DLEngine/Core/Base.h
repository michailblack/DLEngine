#pragma once
#include <memory>

#define BIT(x) (1 << (x))

#define DL_BIND_EVENT_FN(EventFn) [this]<typename EventType>(EventType&& e) { return this->EventFn(std::forward<EventType>(e)); }

template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
