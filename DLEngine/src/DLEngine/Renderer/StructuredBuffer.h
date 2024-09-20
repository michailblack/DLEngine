#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Buffer.h"

namespace DLEngine
{
    enum class BufferViewType
    {
        GPU_READ_CPU_WRITE,
        GPU_READ_WRITE,
    };

    enum BufferMiscFlag
    {
        DL_BUFFER_MISC_FLAG_DRAWINDIRECT_ARGS = BIT(0),
    };

    class StructuredBuffer
    {
    public:
        virtual ~StructuredBuffer() = default;

        virtual Buffer Map() = 0;
        virtual void Unmap() = 0;

        virtual uint32_t GetElementsCount() const noexcept = 0;

        virtual BufferViewType GetViewType() const noexcept = 0;

        static Ref<StructuredBuffer> Create(size_t structureSize, uint32_t elementsCount, BufferViewType viewType = BufferViewType::GPU_READ_CPU_WRITE);
    };

    struct PrimitiveBuffer
    {
    public:
        virtual ~PrimitiveBuffer() = default;

        virtual Buffer Map() = 0;
        virtual void Unmap() = 0;

        virtual uint32_t GetElementsCount() const noexcept = 0;

        virtual BufferViewType GetViewType() const noexcept = 0;

        static Ref<PrimitiveBuffer> Create(uint32_t elementsCount, BufferViewType viewType = BufferViewType::GPU_READ_WRITE, uint32_t bufferMiscFlags = 0u);
    };

    struct BufferViewSpecification
    {
        uint32_t FirstElementIndex{ 0u };
        uint32_t ElementCount{ 0u };

        bool operator==(const BufferViewSpecification& other) const noexcept { return memcmp(this, &other, sizeof(BufferViewSpecification)) == 0u; }
    };
}