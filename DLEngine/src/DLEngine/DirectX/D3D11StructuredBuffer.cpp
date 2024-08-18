#include "dlpch.h"
#include "D3D11StructuredBuffer.h"

#include "DLEngine/DirectX/D3D11Context.h"

namespace DLEngine
{
    D3D11StructuredBuffer::D3D11StructuredBuffer(size_t structureSize, uint32_t elementsCount)
        : m_StructureSize(structureSize), m_ElementsCount(elementsCount)
    {
        DL_ASSERT(m_StructureSize > 0u, "Structure size must be greater than 0");
        DL_ASSERT(m_ElementsCount > 0u, "Elements count must be greater than 0");

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<uint32_t>(m_StructureSize) * m_ElementsCount;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.StructureByteStride = static_cast<uint32_t>(m_StructureSize);

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateBuffer(&bufferDesc, nullptr, &m_D3D11StructuredBuffer));
    }

    Buffer D3D11StructuredBuffer::Map()
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDeviceContext4()->Map(
            m_D3D11StructuredBuffer.Get(),
            0u,
            D3D11_MAP_WRITE_DISCARD,
            0u,
            &mappedSubresource
        ));

        return Buffer{ mappedSubresource.pData, m_StructureSize * static_cast<size_t>(m_ElementsCount) };
    }

    void D3D11StructuredBuffer::Unmap()
    {
        D3D11Context::Get()->GetDeviceContext4()->Unmap(m_D3D11StructuredBuffer.Get(), 0u);
    }

    D3D11StructuredBuffer::D3D11SRV D3D11StructuredBuffer::GetD3D11ShaderResourceView(const BufferViewSpecification& specification) const
    {
        const auto& it{ m_D3D11ShaderResourceViewCache.find(specification) };
        if (it != m_D3D11ShaderResourceViewCache.end())
            return it->second;

        D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        srvDesc.BufferEx.FirstElement = specification.FirstElementIndex;
        srvDesc.BufferEx.NumElements = specification.ElementCount;
        srvDesc.BufferEx.Flags = 0u;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> srv{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateShaderResourceView1(m_D3D11StructuredBuffer.Get(), &srvDesc, &srv));

        m_D3D11ShaderResourceViewCache[specification] = srv;

        return srv;
    }

}