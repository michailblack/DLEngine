#pragma once
#include "DLEngine/Renderer/VertexBuffer.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11VertexBuffer : public VertexBuffer
    {
    public:
        D3D11VertexBuffer(const VertexBufferLayout& layout, const Buffer& buffer, VertexBufferUsage usage);
        D3D11VertexBuffer(const VertexBufferLayout& layout, size_t size, VertexBufferUsage usage);
        
        virtual ~D3D11VertexBuffer() = default;

        Buffer Map() override;
        void Unmap() override;

        const VertexBufferLayout& GetLayout() const noexcept override { return m_Layout; }
        size_t GetSize() const noexcept override { return m_Size; }

        Microsoft::WRL::ComPtr<ID3D11Buffer> GetD3D11VertexBuffer() const noexcept { return m_D3D11VertexBuffer; }

    private:
        VertexBufferLayout m_Layout;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_D3D11VertexBuffer;
        size_t m_Size;
        VertexBufferUsage m_Usage;
    };
}

