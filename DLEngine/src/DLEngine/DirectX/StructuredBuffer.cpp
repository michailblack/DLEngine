#include "dlpch.h"
#include "StructuredBuffer.h"

#include "DLEngine/DirectX/RenderCommand.h"

namespace DLEngine
{
    void StructuredBuffer::Create(const StructuredBufferDesc& desc)
    {
        DL_ASSERT(desc.StructureSize > 0u, "Structure size can't be 0");
        DL_ASSERT(desc.Count > 0u, "Can't allocate structured buffer for less than 1 element");

        m_StructureSize = desc.StructureSize;
        m_Count = desc.Count;

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<uint32_t>(m_StructureSize) * m_Count;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = static_cast<uint32_t>(m_StructureSize);

        D3D11_SUBRESOURCE_DATA structuredBufferData{};
        structuredBufferData.pSysMem = desc.Data;
        structuredBufferData.SysMemPitch = 0u;
        structuredBufferData.SysMemSlicePitch = 0u;

        D3D11_SUBRESOURCE_DATA* pStructuredBufferData = desc.Data ? &structuredBufferData : nullptr;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&bufferDesc, pStructuredBufferData, &m_Handle));
    }

    void StructuredBuffer::Resize(uint32_t count)
    {
        DL_ASSERT(count > 0u, "Can't allocate structured buffer for less than 1 element");

        if (m_Count == count)
            return;

        m_Count = count;

        m_Handle.Reset();

        StructuredBufferDesc desc{};
        desc.StructureSize = m_StructureSize;
        desc.Count = m_Count;

        Create(desc);
    }

    void* StructuredBuffer::Map() const
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        DL_THROW_IF_HR(D3D::GetDeviceContext4()->Map(
            m_Handle.Get(),
            0u,
            D3D11_MAP_WRITE_DISCARD,
            0u,
            &mappedSubresource
        ));

        return mappedSubresource.pData;
    }

    void StructuredBuffer::Unmap() const
    {
        D3D::GetDeviceContext4()->Unmap(m_Handle.Get(), 0u);
    }

    void RStructuredBuffer::Create(const StructuredBuffer& structuredBuffer)
    {
        m_StructuredBuffer = structuredBuffer;
        m_SRV = RenderCommand::CreateShaderResourceView(structuredBuffer);
    }

    void RStructuredBuffer::Resize(uint32_t count)
    {
        m_StructuredBuffer.Resize(count);
        m_SRV.Reset();
        m_SRV = RenderCommand::CreateShaderResourceView(m_StructuredBuffer);
    }
}
