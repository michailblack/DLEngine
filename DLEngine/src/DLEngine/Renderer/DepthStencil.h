#pragma once
#include "DLEngine/Renderer/Common.h"

namespace DLEngine
{
    struct DepthStencilSpecification
    {
        CompareOperator CompareOp{ CompareOperator::None };
        bool DepthTest{ true };
        bool DepthWrite{ true };

        bool operator==(const DepthStencilSpecification& other) const noexcept
        {
            return CompareOp == other.CompareOp && DepthTest == other.DepthTest && DepthWrite == other.DepthWrite;
        }
    };

    struct DepthStencilSpecificationHash
    {
        std::size_t operator()(const DepthStencilSpecification& specification) const noexcept
        {
            return std::hash<CompareOperator>{}(specification.CompareOp) ^
                std::hash<bool>{}(specification.DepthTest) ^
                std::hash<bool>{}(specification.DepthWrite);
        };
    };
}