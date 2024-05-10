#pragma once
#include "DLEngine/Core/DLWin.h"
#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class IndexBuffer
    {
    public:
        IndexBuffer(const std::vector<uint32_t>& indices);

        void Bind();

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
    };
}

