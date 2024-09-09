#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class ConstantBuffer
    {
        friend class RenderCommand;
    public:
        void Create(size_t bufferSize, const void* data = nullptr);
        void Set(const void* data) const;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_Handle{};
        size_t m_BufferSize{ 0u };
    };
}
