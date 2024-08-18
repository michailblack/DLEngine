#pragma once
#include "DLEngine/Renderer/ConstantBuffer.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11ConstantBuffer : public ConstantBuffer
    {
    public:
        D3D11ConstantBuffer(size_t size);

        void SetData(const Buffer& buffer) override;

        const Buffer& GetLocalData() const noexcept override { return m_LocalData; }

        Microsoft::WRL::ComPtr<ID3D11Buffer> GetD3D11ConstantBuffer() const noexcept { return m_D3D11ConstantBuffer; }

    private:
        Buffer m_LocalData;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_D3D11ConstantBuffer;
    };
}

