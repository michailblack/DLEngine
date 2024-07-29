#include "dlpch.h"
#include "StructuredBuffer.h"

#include "DLEngine/DirectX/RenderCommand.h"

namespace DLEngine
{
    void StructuredBuffer::Create(size_t structureSize, uint32_t count, const void* data)
    {
        DL_ASSERT(structureSize > 0u, "Structure size can't be 0");
        DL_ASSERT(count > 0u, "Can't allocate structured buffer for less than 1 element");

        m_StructureSize = structureSize;
        m_Count = count;

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<uint32_t>(m_StructureSize) * m_Count;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = static_cast<uint32_t>(m_StructureSize);

        D3D11_SUBRESOURCE_DATA structuredBufferData{};
        structuredBufferData.pSysMem = data;
        structuredBufferData.SysMemPitch = 0u;
        structuredBufferData.SysMemSlicePitch = 0u;

        D3D11_SUBRESOURCE_DATA* pStructuredBufferData = data ? &structuredBufferData : nullptr;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&bufferDesc, pStructuredBufferData, &m_Handle));

        CreateSRV();
    }

    void StructuredBuffer::Resize(uint32_t count)
    {
        DL_ASSERT(count > 0u, "Can't allocate structured buffer for less than 1 element");

        if (m_Count == count)
            return;

        m_Count = count;

        m_Handle.Reset();

        Create(m_StructureSize, m_Count);
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

    void StructuredBuffer::Unmap() const noexcept
    {
        D3D::GetDeviceContext4()->Unmap(m_Handle.Get(), 0u);
    }

    void StructuredBuffer::CreateSRV()
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0u;
        srvDesc.Buffer.NumElements = m_Count;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateShaderResourceView1(m_Handle.Get(), &srvDesc, &m_SRV));
    }
}
