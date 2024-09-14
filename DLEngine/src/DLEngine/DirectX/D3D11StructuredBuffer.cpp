#include "dlpch.h"
#include "D3D11StructuredBuffer.h"

#include "DLEngine/DirectX/D3D11Context.h"

namespace DLEngine
{
    D3D11StructuredBuffer::D3D11StructuredBuffer(size_t structureSize, uint32_t elementsCount, BufferViewType viewType)
        : m_StructureSize(structureSize), m_ElementsCount(elementsCount), m_ViewType(viewType)
    {
        DL_ASSERT(m_StructureSize > 0u, "Structure size must be greater than 0");
        DL_ASSERT(m_ElementsCount > 0u, "Elements count must be greater than 0");

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<uint32_t>(m_StructureSize) * m_ElementsCount;
        bufferDesc.Usage = m_ViewType == BufferViewType::GPU_READ_WRITE ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
        
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        if (m_ViewType == BufferViewType::GPU_READ_WRITE)
            bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

        bufferDesc.StructureByteStride = static_cast<uint32_t>(m_StructureSize);

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateBuffer(&bufferDesc, nullptr, &m_D3D11StructuredBuffer));
    }

    Buffer D3D11StructuredBuffer::Map()
    {
        DL_ASSERT(m_ViewType == BufferViewType::GPU_READ_CPU_WRITE, "Structured buffer must be GPU_READ_CPU_WRITE to be mapped");

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

    D3D11StructuredBuffer::D3D11UAV D3D11StructuredBuffer::GetD3D11UnorderedAccessView(const BufferViewSpecification& specification) const
    {
        const auto& it{ m_D3D11UnorderedAccessViewCache.find(specification) };
        if (it != m_D3D11UnorderedAccessViewCache.end())
            return it->second;

        D3D11_UNORDERED_ACCESS_VIEW_DESC1 uavDesc{};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = specification.FirstElementIndex;
        uavDesc.Buffer.NumElements = specification.ElementCount;
        uavDesc.Buffer.Flags = 0u;

        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView1> uav{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateUnorderedAccessView1(m_D3D11StructuredBuffer.Get(), &uavDesc, &uav));

        m_D3D11UnorderedAccessViewCache[specification] = uav;

        return uav;
    }

    D3D11PrimitiveBuffer::D3D11PrimitiveBuffer(uint32_t elementsCount, BufferViewType viewType, uint32_t bufferMiscFlags)
        : m_ElementsCount(elementsCount), m_ViewType(viewType)
    {
        DL_ASSERT(m_ElementsCount > 0u, "Elements count must be greater than 0");

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<uint32_t>(m_ElementSize) * m_ElementsCount;
        bufferDesc.Usage = m_ViewType == BufferViewType::GPU_READ_WRITE ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;

        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        if (m_ViewType == BufferViewType::GPU_READ_WRITE)
            bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

        bufferDesc.CPUAccessFlags = m_ViewType == BufferViewType::GPU_READ_CPU_WRITE ? D3D11_CPU_ACCESS_WRITE : 0u;

        if (bufferMiscFlags & DL_BUFFER_MISC_FLAG_DRAWINDIRECT_ARGS)
            bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateBuffer(&bufferDesc, nullptr, &m_D3D11PrimitiveBuffer));
    }

    Buffer D3D11PrimitiveBuffer::Map()
    {
        DL_ASSERT(m_ViewType == BufferViewType::GPU_READ_CPU_WRITE, "Primitive buffer must be GPU_READ_CPU_WRITE to be mapped");

        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDeviceContext4()->Map(
            m_D3D11PrimitiveBuffer.Get(),
            0u,
            D3D11_MAP_WRITE_DISCARD,
            0u,
            &mappedSubresource
        ));

        return Buffer{ mappedSubresource.pData, m_ElementSize * static_cast<size_t>(m_ElementsCount) };
    }

    void D3D11PrimitiveBuffer::Unmap()
    {
        D3D11Context::Get()->GetDeviceContext4()->Unmap(m_D3D11PrimitiveBuffer.Get(), 0u);
    }

    D3D11PrimitiveBuffer::D3D11SRV D3D11PrimitiveBuffer::GetD3D11ShaderResourceView(const BufferViewSpecification& specification) const
    {
        const auto& it{ m_D3D11ShaderResourceViewCache.find(specification) };
        if (it != m_D3D11ShaderResourceViewCache.end())
            return it->second;

        D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_UINT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        srvDesc.BufferEx.FirstElement = specification.FirstElementIndex;
        srvDesc.BufferEx.NumElements = specification.ElementCount;
        srvDesc.BufferEx.Flags = 0u;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> srv{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateShaderResourceView1(m_D3D11PrimitiveBuffer.Get(), &srvDesc, &srv));

        m_D3D11ShaderResourceViewCache[specification] = srv;

        return srv;
    }

    D3D11PrimitiveBuffer::D3D11UAV D3D11PrimitiveBuffer::GetD3D11UnorderedAccessView(const BufferViewSpecification& specification) const
    {
        const auto& it{ m_D3D11UnorderedAccessViewCache.find(specification) };
        if (it != m_D3D11UnorderedAccessViewCache.end())
            return it->second;

        D3D11_UNORDERED_ACCESS_VIEW_DESC1 uavDesc{};
        uavDesc.Format = DXGI_FORMAT_R32_UINT;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = specification.FirstElementIndex;
        uavDesc.Buffer.NumElements = specification.ElementCount;
        uavDesc.Buffer.Flags = 0u;

        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView1> uav{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateUnorderedAccessView1(m_D3D11PrimitiveBuffer.Get(), &uavDesc, &uav));

        m_D3D11UnorderedAccessViewCache[specification] = uav;

        return uav;
    }

}