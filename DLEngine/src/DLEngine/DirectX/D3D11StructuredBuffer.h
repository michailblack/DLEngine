#pragma once
#include "DLEngine/Renderer/StructuredBuffer.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11StructuredBuffer : public StructuredBuffer
    {
        using D3D11SRV = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1>;
    
    public:
        D3D11StructuredBuffer(size_t structureSize, uint32_t elementsCount);

        Buffer Map() override;
        void Unmap() override;

        uint32_t GetElementsCount() const noexcept override { return m_ElementsCount; }

        D3D11SRV GetD3D11ShaderResourceView(const BufferViewSpecification& specification) const;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_D3D11StructuredBuffer{};
        mutable std::unordered_map<BufferViewSpecification, D3D11SRV, BufferViewSpecificationHash> m_D3D11ShaderResourceViewCache;
        size_t m_StructureSize{ 0u };
        uint32_t m_ElementsCount{ 0u };
    };
}