#pragma once
#include "DLEngine/Math/Vec4.h"

#include "DLEngine/Renderer/RendererEnums.h"

namespace DLEngine
{
    struct SamplerSpecification
    {
        Math::Vec4 BorderColor{ 0.0f, 0.0f, 0.0f, 0.0f };
        TextureAddress Address{ TextureAddress::None };
        TextureFilter Filter{ TextureFilter::None };
        CompareOperator CompareOp{ CompareOperator::None };

        SamplerSpecification() noexcept = default;
        
        SamplerSpecification(TextureAddress address, TextureFilter filter, CompareOperator compareOp) noexcept
            : Address(address), Filter(filter), CompareOp(compareOp)
        {}

        bool operator==(const SamplerSpecification& other) const noexcept { return memcmp(this, &other, sizeof(SamplerSpecification)) == 0; }
    };
}