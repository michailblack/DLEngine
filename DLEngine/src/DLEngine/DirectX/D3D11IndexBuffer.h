#pragma once
#include "DLEngine/Renderer/IndexBuffer.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11IndexBuffer : public IndexBuffer
    {
    public:
        D3D11IndexBuffer(const Buffer& buffer);

        ~D3D11IndexBuffer() override = default;

        Microsoft::WRL::ComPtr<ID3D11Buffer> GetD3D11IndexBuffer() const noexcept { return m_D3D11IndexBuffer; }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_D3D11IndexBuffer{};
    };
}

