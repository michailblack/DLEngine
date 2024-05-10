#pragma once
#include "DLEngine/Core/DLWin.h"

#include "DLEngine/DirectX/Shaders.h"
#include "DLEngine/DirectX/BufferLayout.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class InputLayout
    {
    public:
        InputLayout() = default;

        void Bind();

        void AppendVertexBuffer(const BufferLayout& bufferLayout, D3D11_INPUT_CLASSIFICATION inputSlotClass) noexcept;
        void Construct(const Ref<VertexShader>& vertexShader);

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
        std::vector<D3D11_INPUT_ELEMENT_DESC> m_InputElementDescs;
        uint32_t m_Slot{ 0u };
    };
}
