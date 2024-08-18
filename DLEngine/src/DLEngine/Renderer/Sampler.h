#pragma once
#include "DLEngine/Renderer/Common.h"

namespace DLEngine
{
    enum class TextureAddress
    {
        None = 0,
        Wrap,
        Clamp,
    };

    enum class TextureFilter
    {
        None = 0,
        Nearest,
        Trilinear,
        Anisotropic8,
    };

    struct SamplerSpecification
    {
        TextureAddress Address{ TextureAddress::None };
        TextureFilter Filter{ TextureFilter::None };
        CompareOperator CompareOp{ CompareOperator::None };

        bool operator==(const SamplerSpecification& other) const noexcept
        {
            return Address == other.Address && Filter == other.Filter && CompareOp == other.CompareOp;
        }
    };

    struct SamplerSpecificationHash
    {
        std::size_t operator()(const SamplerSpecification& specification) const noexcept
        {
            return std::hash<TextureAddress>{}(specification.Address) ^
                std::hash<TextureFilter>{}(specification.Filter) ^
                std::hash<CompareOperator>{}(specification.CompareOp);
        }
    };
}