#pragma once
#include "DLEngine/Core/Assert.h"

namespace DLEngine
{
    struct Buffer
    {
        void* Data{ nullptr };
        size_t Size{ 0u };

        Buffer() = default;

        Buffer(const void* data, size_t size)
            : Data(const_cast<void*>(data)), Size(size)
        {}

        static Buffer Copy(const Buffer& other)
        {
            Buffer buffer;
            buffer.Allocate(other.Size);
            memcpy_s(buffer.Data, buffer.Size, other.Data, other.Size);
            return buffer;
        }

        static Buffer Copy(const void* data, size_t size)
        {
            Buffer buffer;
            buffer.Allocate(size);
            memcpy_s(buffer.Data, buffer.Size, data, size);
            return buffer;
        }

        void Allocate(size_t size)
        {
            delete[] static_cast<uint8_t*>(Data);
            Data = nullptr;
            Size = 0u;

            if (size == 0u)
                return;

            Data = new uint8_t[size];
            Size = size;
        }

        void Release()
        {
            delete[] reinterpret_cast<uint8_t*>(Data);
            Data = nullptr;
            Size = 0u;
        }

        void Write(const void* data, size_t size, size_t offset = 0u)
        {
            DL_ASSERT(offset + size <= Size, "Buffer overflow");

            memcpy_s(static_cast<uint8_t*>(Data) + offset, Size - offset, data, size);
        }

        template <typename T>
        T& Read(size_t offset = 0u) noexcept
        {
            DL_ASSERT(offset + sizeof(T) <= Size, "Buffer overflow");

            return *reinterpret_cast<T*>(static_cast<uint8_t*>(Data) + offset);
        }

        template <typename T>
        const T& Read(size_t offset = 0u) const noexcept
        {
            DL_ASSERT(offset + sizeof(T) <= Size, "Buffer overflow");

            return *reinterpret_cast<const T*>(static_cast<uint8_t*>(Data) + offset);
        }

        const void* ReadRaw(size_t offset = 0u) const noexcept
        {
            DL_ASSERT(offset < Size, "Buffer overflow");

            return static_cast<uint8_t*>(Data) + offset;
        }

        template <typename T>
        T* As() noexcept { return reinterpret_cast<T*>(Data); }

        template <typename T>
        const T* As() const noexcept { return reinterpret_cast<const T*>(Data); }

        operator bool() const noexcept { return Data != nullptr && Size > 0u; }
    };
}