﻿#pragma once
#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/BufferLayout.h"

namespace DLEngine
{
    class VertexShader;

    class InputLayout
    {
    public:
        void AppendVertexBuffer(const BufferLayout& bufferLayout, D3D11_INPUT_CLASSIFICATION inputSlotClass) noexcept;
        void Create(const VertexShader& vertexShader);

        void Bind() const noexcept;

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout{ nullptr };
        std::vector<D3D11_INPUT_ELEMENT_DESC> m_InputElementDescs{};
        uint32_t m_Slot{ 0u };
    };
}
