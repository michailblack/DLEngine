#pragma once
#include "DLEngine/Math/Vec4.h"

#include "DLEngine/Renderer/Common.h"

namespace DLEngine
{
    enum class TextureAddress
    {
        None = 0,
        Wrap,
        Clamp,
        Border,
    };

    enum class TextureFilter
    {
        None = 0,
        Nearest,
        Trilinear,
        Anisotropic8,
        BilinearCmp,
    };

    struct SamplerSpecification
    {
        Math::Vec4 BorderColor{ 0.0f, 0.0f, 0.0f, 0.0f };
        TextureAddress Address{ TextureAddress::None };
        TextureFilter Filter{ TextureFilter::None };
        CompareOperator CompareOp{ CompareOperator::None };

        explicit SamplerSpecification() noexcept = default;
        explicit SamplerSpecification(TextureAddress address, TextureFilter filter, CompareOperator compareOp) noexcept
            : Address(address), Filter(filter), CompareOp(compareOp)
        {}

        bool operator==(const SamplerSpecification& other) const noexcept
        {
            return BorderColor == other.BorderColor && Address == other.Address && Filter == other.Filter && CompareOp == other.CompareOp;
        }
    };

    struct SamplerSpecificationHash
    {
        std::size_t operator()(const SamplerSpecification& specification) const noexcept
        {
            return std::hash<float>{}(specification.BorderColor.x) ^
                std::hash<float>{}(specification.BorderColor.y) ^
                std::hash<float>{}(specification.BorderColor.z) ^
                std::hash<float>{}(specification.BorderColor.w) ^
                std::hash<TextureAddress>{}(specification.Address) ^
                std::hash<TextureFilter>{}(specification.Filter) ^
                std::hash<CompareOperator>{}(specification.CompareOp);
        }
    };
}