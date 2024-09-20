#pragma once
#include "DLEngine/Renderer/Framebuffer.h"
#include "DLEngine/Renderer/RendererEnums.h"
#include "DLEngine/Renderer/Shader.h"
#include "DLEngine/Renderer/StructuredBuffer.h"

namespace DLEngine
{
    struct RasterizerSpecification
    {
        FillMode Fill{ FillMode::Solid };
        CullMode Cull{ CullMode::Back  };
        int32_t DepthBias{ 0 };
        float SlopeScaledDepthBias{ 0.0f };

        bool operator==(const RasterizerSpecification& other) const noexcept { return memcmp(this, &other, sizeof(RasterizerSpecification)) == 0; }
    };

    struct StencilFaceSpecification
    {
        CompareOperator CompareOp  { CompareOperator::Always };
        StencilOperator FailOp     { StencilOperator::Keep   };
        StencilOperator DepthFailOp{ StencilOperator::Keep   };
        StencilOperator PassOp     { StencilOperator::Keep   };
    };

    struct DepthStencilSpecification
    {
        bool DepthTest { true };
        bool DepthWrite{ true };
        CompareOperator DepthCompareOp{ CompareOperator::None };

        bool StencilTest{ false };
        uint8_t StencilReadMask { 0xFF };
        uint8_t StencilWriteMask{ 0xFF };

        StencilFaceSpecification FrontFace;
        StencilFaceSpecification BackFace;

        bool operator==(const DepthStencilSpecification& other) const noexcept { return memcmp(this, &other, sizeof(DepthStencilSpecification)) == 0; }
    };

    struct BlendSpecification
    {
        std::map<uint32_t, BlendType> BlendTypes;
        bool IndependentBlend{ false };
        bool AlphaToCoverage{ false };

        bool operator==(const BlendSpecification& other) const noexcept { return memcmp(this, &other, sizeof(BlendSpecification)) == 0; }
    };

    struct PipelineSpecification
    {
        std::string DebugName;

        Ref<Shader> Shader;
        Ref<Framebuffer> TargetFramebuffer;
        PrimitiveTopology Topology{ PrimitiveTopology::TrianglesList };
        DepthStencilSpecification DepthStencilState;
        RasterizerSpecification RasterizerState;
        BlendSpecification BlendState;
    };
    
    class Pipeline
    {
    public:
        virtual ~Pipeline() = default;

        virtual const PipelineSpecification& GetSpecification() const noexcept = 0;

        virtual void SetRWStructuredBuffer(uint32_t bindPoint, const Ref<StructuredBuffer>& structuredBuffer) noexcept = 0;
        virtual void SetRWStructuredBufferView(uint32_t bindPoint, const BufferViewSpecification& viewSpecification) noexcept = 0;

        virtual void SetRWPrimitiveBuffer(uint32_t bindPoint, const Ref<PrimitiveBuffer>& primitiveBuffer) noexcept = 0;
        virtual void SetRWPrimitiveBufferView(uint32_t bindPoint, const BufferViewSpecification& viewSpecification) noexcept = 0;

        virtual const std::map<uint32_t, Ref<StructuredBuffer>>& GetRWStructuredBuffers() const noexcept = 0;
        virtual const std::map<uint32_t, BufferViewSpecification>& GetRWStructuredBufferViews() const noexcept = 0;

        virtual const std::map<uint32_t, Ref<PrimitiveBuffer>>& GetRWPrimitiveBuffers() const noexcept = 0;
        virtual const std::map<uint32_t, BufferViewSpecification>& GetRWPrimitiveBufferViews() const noexcept = 0;

        static Ref<Pipeline> Create(const PipelineSpecification& specification);
    };
}