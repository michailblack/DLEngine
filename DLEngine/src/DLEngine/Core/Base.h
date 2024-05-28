#pragma once
#include <memory>

#include "DLEngine/Core/Log.h"

#ifdef DL_ENABLE_ASSERTS
    #define DL_ASSERT(x, ...) { if(!(x)) { DL_LOG_CRITICAL("Assertion Failed: {}", __VA_ARGS__); __debugbreak(); } }
    #define DL_ASSERT_NOINFO(x) if(!(x)) __debugbreak()

#ifdef DL_DEBUG
    #define DL_ASSERT_EXPR(expr) { if(!(expr)) { DL_LOG_CRITICAL("Assertion Failed: {}", #expr); __debugbreak(); } }
#else
    #define DL_ASSERT_EXPR(expr) (expr)
#endif

#else
    #define DL_ASSERT(x, ...)
    #define DL_ASSERT_NOINFO(x)
    #define DL_ASSERT_EXPR(expr) (expr)
#endif


#define BIT(x) (1 << (x))

#define STRINGIFY(x) #x

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
