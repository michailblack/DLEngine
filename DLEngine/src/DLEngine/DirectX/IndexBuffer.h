#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class IndexBuffer
    {
    public:
        void Create(const std::vector<uint32_t>& indices);

        void Bind() const;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
    };
}

