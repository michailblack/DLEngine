#include "dlpch.h"
#include "ConstantBuffer.h"

namespace DLEngine
{
    void ConstantBuffer::Create(size_t bufferSize, const void* data)
    {
        DL_ASSERT(bufferSize > 0u, "Constant buffer can't be empty");

        m_BufferSize = bufferSize;

        D3D11_BUFFER_DESC constantBufferDesc{};
        constantBufferDesc.ByteWidth = static_cast<UINT>(bufferSize);
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantBufferDesc.MiscFlags = 0u;
        constantBufferDesc.StructureByteStride = 0u;

        D3D11_SUBRESOURCE_DATA constantBufferData{};
        constantBufferData.pSysMem = data;
        constantBufferData.SysMemPitch = 0u;
        constantBufferData.SysMemSlicePitch = 0u;

        D3D11_SUBRESOURCE_DATA* pConstantBufferData = data ? &constantBufferData : nullptr;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&constantBufferDesc, pConstantBufferData, &m_Handle));
    }

    void ConstantBuffer::Set(const void* data) const
    {
        DL_ASSERT(m_BufferSize > 0u && m_Handle, "Constant buffer is not created");
        DL_ASSERT(data, "Data can't be nullptr");

        const auto& deviceContext{ D3D::GetDeviceContext4() };

        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        DL_THROW_IF_HR(deviceContext->Map(
            m_Handle.Get(),
            0u,
            D3D11_MAP_WRITE_DISCARD,
            0u,
            &mappedSubresource
        ));
        memcpy_s(mappedSubresource.pData, m_BufferSize, data, m_BufferSize);
        deviceContext->Unmap(m_Handle.Get(), 0u);
    }
}
