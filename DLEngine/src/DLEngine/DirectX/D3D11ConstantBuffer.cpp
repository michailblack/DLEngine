#include "dlpch.h"
#include "D3D11ConstantBuffer.h"

#include "DLEngine/DirectX/D3D11Context.h"

namespace DLEngine
{
    D3D11ConstantBuffer::D3D11ConstantBuffer(size_t size)
    {
        DL_ASSERT(size > 0u, "Buffer size must be greater than 0");

        m_LocalData.Allocate(size);

        D3D11_BUFFER_DESC constantBufferDesc{};
        constantBufferDesc.ByteWidth = static_cast<UINT>(size);
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantBufferDesc.MiscFlags = 0u;
        constantBufferDesc.StructureByteStride = 0u;

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateBuffer(&constantBufferDesc, nullptr, &m_D3D11ConstantBuffer));
    }

    void D3D11ConstantBuffer::SetData(const Buffer& buffer)
    {
        DL_ASSERT(buffer, "Buffer must be valid");
        DL_ASSERT(buffer.Size == m_LocalData.Size, "Buffer size must match the constant buffer size");

        m_LocalData = Buffer::Copy(buffer);

        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDeviceContext4()->Map(
            m_D3D11ConstantBuffer.Get(),
            0u,
            D3D11_MAP_WRITE_DISCARD,
            0u,
            &mappedSubresource
        ));
        memcpy_s(mappedSubresource.pData, m_LocalData.Size, buffer.Data, m_LocalData.Size);
        D3D11Context::Get()->GetDeviceContext4()->Unmap(m_D3D11ConstantBuffer.Get(), 0u);
    }
}