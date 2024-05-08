#pragma once
#include "DLEngine/DirectX/IBindable.h"
#include "DLEngine/DirectX/Shaders.h"
#include "DLEngine/DirectX/BufferLayout.h"

namespace DLEngine
{
    class InputLayout
        : public IBindable
    {
    public:
        InputLayout() = default;
        ~InputLayout() override = default;

        void Bind() override;

        void AppendVertexBuffer(const BufferLayout& bufferLayout, D3D11_INPUT_CLASSIFICATION inputSlotClass) noexcept;
        void Construct(const Ref<VertexShader>& vertexShader);

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
        std::vector<D3D11_INPUT_ELEMENT_DESC> m_InputElementDescs;
        uint32_t m_Slot{ 0u };
    };
}
