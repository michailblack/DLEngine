#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Buffer.h"

namespace DLEngine
{
    class StructuredBuffer
    {
    public:
        virtual ~StructuredBuffer() = default;

        virtual Buffer Map() = 0;
        virtual void Unmap() = 0;

        virtual uint32_t GetElementsCount() const noexcept = 0;

        static Ref<StructuredBuffer> Create(size_t structureSize, uint32_t elementsCount);
    };

    struct BufferViewSpecification
    {
        uint32_t FirstElementIndex{ 0u };
        uint32_t ElementCount{ 0u };

        bool operator==(const BufferViewSpecification& other) const noexcept
        {
            return FirstElementIndex == other.FirstElementIndex && ElementCount == other.ElementCount;
        }
    };

    struct BufferViewSpecificationHash
    {
        std::size_t operator()(const BufferViewSpecification& specification) const noexcept
        {
            return std::hash<uint32_t>{}(specification.FirstElementIndex) ^ std::hash<uint32_t>{}(specification.ElementCount);
        }
    };
}