#pragma once
#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/BufferLayout.h"

namespace DLEngine
{
    class VertexBuffer
    {
        friend class RenderCommand;
    public:
        void Create(const BufferLayout& layout, D3D11_USAGE usage, uint32_t vertexCount = 1u, const void* data = nullptr);

        void Resize(uint32_t vertexCount);

        void* Map() const;
        void Unmap() const;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_Handle{};
        BufferLayout m_BufferLayout{};
        uint32_t m_VertexCount{ 0u };
    };
}
