#pragma once
#include "DLEngine/Renderer/StructuredBuffer.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11StructuredBuffer : public StructuredBuffer
    {
        using D3D11SRV = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1>;
        using D3D11UAV = Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView1>;
    
    public:
        D3D11StructuredBuffer(size_t structureSize, uint32_t elementsCount, BufferViewType viewType);

        Buffer Map() override;
        void Unmap() override;

        uint32_t GetElementsCount() const noexcept override { return m_ElementsCount; }

        BufferViewType GetViewType() const noexcept override { return m_ViewType; }

        D3D11SRV GetD3D11ShaderResourceView(const BufferViewSpecification& specification) const;
        D3D11UAV GetD3D11UnorderedAccessView(const BufferViewSpecification& specification) const;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_D3D11StructuredBuffer{};
        mutable std::unordered_map<BufferViewSpecification, D3D11SRV, ByteBufferHash<BufferViewSpecification>> m_D3D11ShaderResourceViewCache;
        mutable std::unordered_map<BufferViewSpecification, D3D11UAV, ByteBufferHash<BufferViewSpecification>> m_D3D11UnorderedAccessViewCache;
        size_t m_StructureSize{ 0u };
        uint32_t m_ElementsCount{ 0u };
        BufferViewType m_ViewType{ BufferViewType::GPU_READ_CPU_WRITE };
    };

    class D3D11PrimitiveBuffer : public PrimitiveBuffer
    {
        using D3D11Buffer = Microsoft::WRL::ComPtr<ID3D11Buffer>;
        using D3D11SRV = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1>;
        using D3D11UAV = Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView1>;

    public:
        D3D11PrimitiveBuffer(uint32_t elementsCount, BufferViewType viewType, uint32_t bufferMiscFlags);

        Buffer Map() override;
        void Unmap() override;

        uint32_t GetElementsCount() const noexcept override { return m_ElementsCount; }

        BufferViewType GetViewType() const noexcept override { return m_ViewType; }

        D3D11Buffer GetD3D11Buffer() const noexcept { return m_D3D11PrimitiveBuffer; }

        D3D11SRV GetD3D11ShaderResourceView(const BufferViewSpecification& specification) const;
        D3D11UAV GetD3D11UnorderedAccessView(const BufferViewSpecification& specification) const;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_D3D11PrimitiveBuffer{};
        mutable std::unordered_map<BufferViewSpecification, D3D11SRV, ByteBufferHash<BufferViewSpecification>> m_D3D11ShaderResourceViewCache;
        mutable std::unordered_map<BufferViewSpecification, D3D11UAV, ByteBufferHash<BufferViewSpecification>> m_D3D11UnorderedAccessViewCache;
        const size_t m_ElementSize{ 4u };
        uint32_t m_ElementsCount{ 0u };
        BufferViewType m_ViewType{ BufferViewType::GPU_READ_CPU_WRITE };
    };
}