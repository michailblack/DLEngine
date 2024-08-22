#pragma once
#include <cstdint>

namespace DLEngine
{
    enum class FillMode
    {
        Solid,
        Wireframe
    };

    enum class CullMode
    {
        None,
        Front,
        Back
    };

    struct RasterizerSpecification
    {
        FillMode Fill{ FillMode::Solid };
        CullMode Cull{ CullMode::Back };
        int32_t DepthBias{ 0 };
        float SlopeScaledDepthBias{ 0.0f };

        bool operator==(const RasterizerSpecification& other) const noexcept
        {
            return Fill == other.Fill && Cull == other.Cull && DepthBias == other.DepthBias && SlopeScaledDepthBias == other.SlopeScaledDepthBias;
        }
    };

    struct RasterizerSpecificationHash
    {
        std::size_t operator()(const RasterizerSpecification& specification) const noexcept
        {
            return std::hash<FillMode>{}(specification.Fill) ^
                std::hash<CullMode>{}(specification.Cull) ^
                std::hash<int32_t>{}(specification.DepthBias) ^
                std::hash<float>{}(specification.SlopeScaledDepthBias);
        };
    };
}