#pragma once
#include "DLEngine/DirectX/IBindable.h"
#include "DLEngine/DirectX/VertexLayout.h"

namespace DLEngine
{
    class VertexBuffer
        : public IBindable
    {
    public:
        VertexBuffer(const VertexLayout& vertexLayout, const void* data, uint32_t vertexNum);
        ~VertexBuffer() override = default;

        void Bind() override;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
        VertexLayout m_VertexLayout;
        uint32_t m_Stride;
    };
}
