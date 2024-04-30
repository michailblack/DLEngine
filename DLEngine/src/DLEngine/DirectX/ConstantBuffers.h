#pragma once
#include "DLEngine/Core/DLException.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/IBindable.h"

template <typename Data>
class ConstantBuffer
    : public IBindable
{
public:
    ConstantBuffer(const Data& data, uint32_t slot = 0u)
        : m_Slot(slot)
    {
        D3D11_BUFFER_DESC constantBufferDesc {};
        constantBufferDesc.ByteWidth = sizeof(Data);
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantBufferDesc.MiscFlags = 0u;
        constantBufferDesc.StructureByteStride = 0u;

        D3D11_SUBRESOURCE_DATA constantBufferData {};
        constantBufferData.pSysMem = &data;
        constantBufferData.SysMemPitch = 0u;
        constantBufferData.SysMemSlicePitch = 0u;

        DL_THROW_IF(D3D::Get().GetDevice()->CreateBuffer(&constantBufferDesc, &constantBufferData, &m_ConstantBuffer));
    }

    ConstantBuffer(uint32_t slot = 0u)
        : m_Slot(slot)
    {
        D3D11_BUFFER_DESC constantBufferDesc {};
        constantBufferDesc.ByteWidth = sizeof(Data);
        constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantBufferDesc.MiscFlags = 0u;
        constantBufferDesc.StructureByteStride = 0u;

        DL_THROW_IF(D3D::Get().GetDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_ConstantBuffer));
    }

    ~ConstantBuffer() override = default;

    void Set(const Data& data)
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource {};
        DL_THROW_IF(D3D::Get().GetDeviceContext()->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource));
        memcpy_s(mappedSubresource.pData, sizeof(Data), &data, sizeof(data));
        D3D::Get().GetDeviceContext()->Unmap(m_ConstantBuffer.Get(), 0u);
    }

protected:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
    uint32_t m_Slot;
};

template <typename Data>
class VertexConstantBuffer
    : public ConstantBuffer<Data>
{
    using ConstantBuffer<Data>::m_ConstantBuffer;
    using ConstantBuffer<Data>::m_Slot;

public:
    using ConstantBuffer<Data>::ConstantBuffer;

    void Bind() override
    {
        D3D::Get().GetDeviceContext()->VSSetConstantBuffers1(m_Slot, 1u, m_ConstantBuffer.GetAddressOf(), nullptr, nullptr);
    }
};

template <typename Data>
class PixelConstantBuffer
    : public ConstantBuffer<Data>
{
    using ConstantBuffer<Data>::m_ConstantBuffer;
    using ConstantBuffer<Data>::m_Slot;

public:
    using ConstantBuffer<Data>::ConstantBuffer;

    void Bind() override
    {
        D3D::Get().GetDeviceContext()->PSSetConstantBuffers1(m_Slot, 1u, m_ConstantBuffer.GetAddressOf(), nullptr, nullptr);
    }
};
