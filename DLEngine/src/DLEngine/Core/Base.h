#pragma once
#include <memory>
#include <string_view>

#define BIT(x) (1 << (x))

#define STRINGIFY(x) #x

#define DL_BIND_EVENT_FN(EventFn) [this]<typename EventType>(EventType&& e) { return this->EventFn(std::forward<EventType>(e)); }

namespace DLEngine
{
    template <typename T>
    using Ref = std::shared_ptr<T>;
    template <typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename CastT, typename T>
    constexpr Ref<CastT> AsRef(const Ref<T>& ref)
    {
        return std::static_pointer_cast<CastT>(ref);
    }

    template <typename T>
    using Scope = std::unique_ptr<T>;
    template <typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    struct ByteBufferHash
    {
        std::size_t operator()(const T& value) const noexcept
        {
            return std::hash<std::string_view>{}(std::string_view{ reinterpret_cast<const char*>(&value), sizeof(T) });
        };
    };
}
