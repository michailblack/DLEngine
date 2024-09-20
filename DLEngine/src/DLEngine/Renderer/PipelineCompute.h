#pragma once
#include "DLEngine/Renderer/Shader.h"
#include "DLEngine/Renderer/StructuredBuffer.h"

namespace DLEngine
{
    struct PipelineComputeSpecification
    {
        std::string DebugName;

        Ref<Shader> ComputeShader;
    };

    class PipelineCompute
    {
    public:
        virtual ~PipelineCompute() = default;

        virtual const PipelineComputeSpecification& GetSpecification() const noexcept = 0;

        virtual void SetRWStructuredBuffer(uint32_t bindPoint, const Ref<StructuredBuffer>& structuredBuffer) noexcept = 0;
        virtual void SetRWStructuredBufferView(uint32_t bindPoint, const BufferViewSpecification& viewSpecification) noexcept = 0;

        virtual void SetRWPrimitiveBuffer(uint32_t bindPoint, const Ref<PrimitiveBuffer>& primitiveBuffer) noexcept = 0;
        virtual void SetRWPrimitiveBufferView(uint32_t bindPoint, const BufferViewSpecification& viewSpecification) noexcept = 0;

        virtual const std::map<uint32_t, Ref<StructuredBuffer>>& GetRWStructuredBuffers() const noexcept = 0;
        virtual const std::map<uint32_t, BufferViewSpecification>& GetRWStructuredBufferViews() const noexcept = 0;

        virtual const std::map<uint32_t, Ref<PrimitiveBuffer>>& GetRWPrimitiveBuffers() const noexcept = 0;
        virtual const std::map<uint32_t, BufferViewSpecification>& GetRWPrimitiveBufferViews() const noexcept = 0;

        static Ref<PipelineCompute> Create(const PipelineComputeSpecification& specificaton);
    };
}