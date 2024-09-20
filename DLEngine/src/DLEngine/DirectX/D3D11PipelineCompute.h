#pragma once
#include "DLEngine/Renderer/PipelineCompute.h"

namespace DLEngine
{
    class D3D11PipelineCompute : public PipelineCompute
    {
    public:
        D3D11PipelineCompute(const PipelineComputeSpecification& specificaton);

        void SetRWStructuredBuffer(uint32_t bindPoint, const Ref<StructuredBuffer>& structuredBuffer) noexcept override;
        void SetRWStructuredBufferView(uint32_t bindPoint, const BufferViewSpecification& viewSpecification) noexcept override;

        void SetRWPrimitiveBuffer(uint32_t bindPoint, const Ref<PrimitiveBuffer>& primitiveBuffer) noexcept override;
        void SetRWPrimitiveBufferView(uint32_t bindPoint, const BufferViewSpecification& viewSpecification) noexcept override;

        const std::map<uint32_t, Ref<StructuredBuffer>>& GetRWStructuredBuffers() const noexcept override { return m_RWStructuredBuffers; }
        const std::map<uint32_t, BufferViewSpecification>& GetRWStructuredBufferViews() const noexcept override { return m_RWStructuredBufferViews; }

        const std::map<uint32_t, Ref<PrimitiveBuffer>>& GetRWPrimitiveBuffers() const noexcept override { return m_RWPrimitiveBuffers; }
        const std::map<uint32_t, BufferViewSpecification>& GetRWPrimitiveBufferViews() const noexcept override { return m_RWPrimitiveBufferViews; }

        const PipelineComputeSpecification& GetSpecification() const noexcept override { return m_Specification; }

    private:
        PipelineComputeSpecification m_Specification;

        std::map<uint32_t, Ref<StructuredBuffer>> m_RWStructuredBuffers;
        std::map<uint32_t, BufferViewSpecification> m_RWStructuredBufferViews;

        std::map<uint32_t, Ref<PrimitiveBuffer>> m_RWPrimitiveBuffers;
        std::map<uint32_t, BufferViewSpecification> m_RWPrimitiveBufferViews;
    };
}