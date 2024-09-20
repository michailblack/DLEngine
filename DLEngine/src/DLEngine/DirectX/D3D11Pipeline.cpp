#include "dlpch.h"
#include "D3D11Pipeline.h"

#include "DLEngine/DirectX/D3D11StructuredBuffer.h"

namespace DLEngine
{
    D3D11Pipeline::D3D11Pipeline(const PipelineSpecification& specificaton)
        : m_Specification(specificaton)
    {

    }

    void D3D11Pipeline::SetRWStructuredBuffer(uint32_t bindPoint, const Ref<StructuredBuffer>& structuredBuffer) noexcept
    {
        DL_ASSERT(structuredBuffer->GetViewType() == BufferViewType::GPU_READ_WRITE, "StructuredBuffer must be of type ReadWrite");
        m_RWStructuredBuffers[bindPoint] = structuredBuffer;
    }

    void D3D11Pipeline::SetRWStructuredBufferView(uint32_t bindPoint, const BufferViewSpecification& viewSpecification) noexcept
    {
        m_RWStructuredBufferViews[bindPoint] = viewSpecification;
    }

    void D3D11Pipeline::SetRWPrimitiveBuffer(uint32_t bindPoint, const Ref<PrimitiveBuffer>& primitiveBuffer) noexcept
    {
        DL_ASSERT(primitiveBuffer->GetViewType() == BufferViewType::GPU_READ_WRITE, "PrimitiveBuffer must be of type ReadWrite");
        m_RWPrimitiveBuffers[bindPoint] = primitiveBuffer;
    }

    void D3D11Pipeline::SetRWPrimitiveBufferView(uint32_t bindPoint, const BufferViewSpecification& viewSpecification) noexcept
    {
        m_RWPrimitiveBufferViews[bindPoint] = viewSpecification;
    }

}