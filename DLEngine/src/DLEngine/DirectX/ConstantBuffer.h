#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/DLException.h"

#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    template <typename Data>
    class ConstantBuffer
    {
    public:
        void Create()
        {
            D3D11_BUFFER_DESC constantBufferDesc{};
            constantBufferDesc.ByteWidth = sizeof(Data);
            constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            constantBufferDesc.MiscFlags = 0u;
            constantBufferDesc.StructureByteStride = 0u;

            DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&constantBufferDesc, nullptr, &m_ConstantBuffer));
        }

        void Create(const Data& data)
        {
            D3D11_BUFFER_DESC constantBufferDesc{};
            constantBufferDesc.ByteWidth = sizeof(Data);
            constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            constantBufferDesc.MiscFlags = 0u;
            constantBufferDesc.StructureByteStride = 0u;

            D3D11_SUBRESOURCE_DATA constantBufferData{};
            constantBufferData.pSysMem = &data;
            constantBufferData.SysMemPitch = 0u;
            constantBufferData.SysMemSlicePitch = 0u;

            DL_THROW_IF_HR(D3D::GetDevice5()->CreateBuffer(&constantBufferDesc, &constantBufferData, &m_ConstantBuffer));
        }

        void Bind(uint32_t slot, uint8_t shaderBindFlags) const noexcept
        {
            const auto& deviceContext{ D3D::GetDeviceContext4() };
            
            if (shaderBindFlags & BIND_VS)
                deviceContext->VSSetConstantBuffers1(slot, 1u, m_ConstantBuffer.GetAddressOf(), nullptr, nullptr);

            if (shaderBindFlags & BIND_PS)
                deviceContext->PSSetConstantBuffers1(slot, 1u, m_ConstantBuffer.GetAddressOf(), nullptr, nullptr);

            if (shaderBindFlags & BIND_DS)
                deviceContext->DSSetConstantBuffers1(slot, 1u, m_ConstantBuffer.GetAddressOf(), nullptr, nullptr);

            if (shaderBindFlags & BIND_HS)
                deviceContext->HSSetConstantBuffers1(slot, 1u, m_ConstantBuffer.GetAddressOf(), nullptr, nullptr);

            if (shaderBindFlags & BIND_GS)
                deviceContext->GSSetConstantBuffers1(slot, 1u, m_ConstantBuffer.GetAddressOf(), nullptr, nullptr);
        }

        void Set(const Data& data)
        {
            D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
            DL_THROW_IF_HR(D3D::GetDeviceContext4()->Map(
                m_ConstantBuffer.Get(),
                0u,
                D3D11_MAP_WRITE_DISCARD,
                0u,
                &mappedSubresource
            ));
            memcpy_s(mappedSubresource.pData, sizeof(Data), &data, sizeof(data));
            D3D::GetDeviceContext4()->Unmap(m_ConstantBuffer.Get(), 0u);
        }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer{};
    };
}
