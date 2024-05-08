#pragma once
#include "DLEngine/DirectX/IBindable.h"

namespace DLEngine
{
    class IndexBuffer
        : public IBindable
    {
    public:
        IndexBuffer(const std::vector<uint32_t>& indices);
        ~IndexBuffer() override = default;

        void Bind() override;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
    };
}

