#include "dlpch.h"
#include "InputLayout.h"

#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            DXGI_FORMAT GetD3D11Format(BufferLayout::ShaderDataType type)
            {
                switch (type)
                {
                case BufferLayout::ShaderDataType::Float:  return DXGI_FORMAT_R32_FLOAT;
                case BufferLayout::ShaderDataType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
                case BufferLayout::ShaderDataType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
                
                case BufferLayout::ShaderDataType::Mat4:
                case BufferLayout::ShaderDataType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;

                default: DL_ASSERT(false, "Unknown ShaderDatatType"); return DXGI_FORMAT_UNKNOWN;
                }
            }
        }
    }

    void InputLayout::Bind()
    {
        D3D::GetDeviceContext4()->IASetInputLayout(m_InputLayout.Get());
    }

    void InputLayout::AppendVertexBuffer(const BufferLayout& bufferLayout, D3D11_INPUT_CLASSIFICATION inputSlotClass) noexcept
    {
        DL_ASSERT(!m_InputLayout, "Input Layout is already built");

        m_InputElementDescs.reserve(m_InputElementDescs.size() + bufferLayout.GetElements().size());

        D3D11_INPUT_ELEMENT_DESC elementDesc{};
        elementDesc.InputSlot = m_Slot++;
        elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

        switch (inputSlotClass)
        {
        case D3D11_INPUT_PER_VERTEX_DATA:
            elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            elementDesc.InstanceDataStepRate = 0u;
            break;
        case D3D11_INPUT_PER_INSTANCE_DATA:
            elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
            elementDesc.InstanceDataStepRate = 1u;
            break;
        }

        for (const auto& bufferElement : bufferLayout)
        {
            elementDesc.SemanticName = bufferElement.Semantics.c_str();
            elementDesc.Format = Utils::GetD3D11Format(bufferElement.DataType);

            switch (bufferElement.DataType)
            {
            case BufferLayout::ShaderDataType::Mat4:
            {
                for (uint32_t i{ 0u }; i < 4u; ++i)
                {
                    elementDesc.SemanticIndex = i;
                    m_InputElementDescs.push_back(elementDesc);
                }
            } break;
            default:
                elementDesc.SemanticIndex = 0u;
                m_InputElementDescs.push_back(elementDesc);
                break;
            }

        }
    }

    void InputLayout::Construct(const Ref<VertexShader>& vertexShader)
    {
        DL_ASSERT(!m_InputLayout, "Input Layout is already built")

        D3D::GetDevice5()->CreateInputLayout(
            m_InputElementDescs.data(), static_cast<uint32_t>(m_InputElementDescs.size()),
            vertexShader->GetVertexShaderBlob()->GetBufferPointer(), vertexShader->GetVertexShaderBlob()->GetBufferSize(),
            &m_InputLayout
        );
    }
}
