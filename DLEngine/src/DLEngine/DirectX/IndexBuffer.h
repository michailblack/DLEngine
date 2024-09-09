#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class IndexBuffer
    {
        friend class RenderCommand;
    public:
        void Create(const std::vector<uint32_t>& indices);

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_Handle{};
    };
}

