#include "dlpch.h"
#include "InputLayout.h"

#include "DLEngine/DirectX/D3D.h"

InputLayout::InputLayout(const VertexLayout& vertexLayout, const Ref<VertexShader>& vertexShader)
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs(vertexLayout.GetElementCount());

    for (uint32_t i = 0; i < vertexLayout.GetElementCount(); ++i)
    {
        const auto& elements { vertexLayout.GetElements() };

        inputElementDescs[i] =
        {
            elements[i].Semantics.c_str(),
            0u,
            elements[i].Format,
            elements[i].Slot,
            D3D11_APPEND_ALIGNED_ELEMENT ,
            D3D11_INPUT_PER_VERTEX_DATA,
            0u
        };
    }

    DL_THROW_IF_HR(D3D::Get().GetDevice()->CreateInputLayout(
        inputElementDescs.data(), static_cast<uint32_t>(inputElementDescs.size()),
        vertexShader->GetVertexShaderBlob()->GetBufferPointer(), vertexShader->GetVertexShaderBlob()->GetBufferSize(),
        &m_InputLayout
    ));
}

void InputLayout::Bind()
{
    D3D::Get().GetDeviceContext()->IASetInputLayout(m_InputLayout.Get());
}
