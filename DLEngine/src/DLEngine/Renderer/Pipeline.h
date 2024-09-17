#pragma once
#include "DLEngine/Renderer/Framebuffer.h"
#include "DLEngine/Renderer/Shader.h"

namespace DLEngine
{
    enum class PrimitiveTopology
    {
        None = 0,
        TrianglesList,
        TriangleStrip,
    };

    enum class FillMode
    {
        Solid = 0,
        Wireframe
    };

    enum class CullMode
    {
        None = 0,
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

    enum class BlendState
    {
        None = 0,

        General,
        AlphaToCoverage,

        PremultipliedAlpha,
        Additive,
    };

    struct PipelineSpecification
    {
        std::string DebugName;

        Ref<Shader> Shader;
        Ref<Framebuffer> TargetFramebuffer;
        PrimitiveTopology Topology{ PrimitiveTopology::TrianglesList };
        DepthStencilSpecification DepthStencilState;
        RasterizerSpecification RasterizerState;
        BlendState BlendState{ BlendState::None };
    };
    
    class Pipeline
    {
    public:
        virtual ~Pipeline() = default;

        virtual void SetFramebuffer(const Ref<Framebuffer>& framebuffer) noexcept = 0;

        virtual const PipelineSpecification& GetSpecification() const noexcept = 0;

        static Ref<Pipeline> Create(const PipelineSpecification& specification);
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