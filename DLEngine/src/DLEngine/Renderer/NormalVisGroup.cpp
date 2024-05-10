#include "dlpch.h"
#include "NormalVisGroup.h"

#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    NormalVisGroup::NormalVisGroup()
    {
        const auto& device{ D3D::GetDevice5() };
        
        ShaderSpecification shaderSpec;
        shaderSpec.Name = "NormalVis.vs";
        shaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\NormalVis.vs.hlsl";
        m_VertexShader = CreateRef<VertexShader>(shaderSpec);

        shaderSpec.Name = "NormalVis.ps";
        shaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\NormalVis.ps.hlsl";
        m_PixelShader = CreateRef<PixelShader>(shaderSpec);

        m_InputLayout = CreateRef<InputLayout>();

        static const BufferLayout instanceBufferLayout{
            { "TRANSFORM", BufferLayout::ShaderDataType::Mat4 }
        };
        m_InputLayout->AppendVertexBuffer(instanceBufferLayout, D3D11_INPUT_PER_INSTANCE_DATA);

        m_InstanceBuffer = CreateRef<PerInstanceBuffer<Instance>>(instanceBufferLayout);

        static const BufferLayout vertexBufferLayout{
            { "POSITION"  , BufferLayout::ShaderDataType::Float3 },
            { "NORMAL"    , BufferLayout::ShaderDataType::Float3 },
            { "TANGENT"   , BufferLayout::ShaderDataType::Float3 },
            { "BITANGENT" , BufferLayout::ShaderDataType::Float3 },
            { "TEXCOORDS" , BufferLayout::ShaderDataType::Float2 }
        };
        m_InputLayout->AppendVertexBuffer(vertexBufferLayout, D3D11_INPUT_PER_VERTEX_DATA);
        m_InputLayout->Construct(m_VertexShader);

        D3D11_RASTERIZER_DESC2 rasterizerDesc{};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_NONE;
        rasterizerDesc.FrontCounterClockwise = true;
        rasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = true;
        rasterizerDesc.ScissorEnable = false;
        rasterizerDesc.MultisampleEnable = false;
        rasterizerDesc.AntialiasedLineEnable = false;
        rasterizerDesc.ForcedSampleCount = 0u;
        rasterizerDesc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        DL_THROW_IF_HR(device->CreateRasterizerState2(&rasterizerDesc, &m_RasterizerState));

        m_PerDrawConstantBuffer = CreateRef<ConstantBuffer<PerDraw>>();
    }

    void NormalVisGroup::AddModel(const Ref<Model>& model, const std::vector<Instance>& instances)
    {
        if (instances.empty())
            return;

        PerModel perModel;
        perModel.Model = model;
        perModel.PerMesh.resize(model->GetMeshesCount());

        for (uint32_t i{ 0u }; i < model->GetMeshesCount(); ++i)
        {
            auto& dstMesh{ perModel.PerMesh[i] };

            for (const auto& instance : instances)
                dstMesh.Instances.push_back(instance);
        }

        m_Models.push_back(perModel);
    }

    void NormalVisGroup::Render()
    {
        const auto& deviceContext{ DLEngine::D3D::GetDeviceContext4() };

        UpdateInstanceBuffer();

        m_InputLayout->Bind();

        m_VertexShader->Bind();
        m_PixelShader->Bind();

        deviceContext->RSSetState(m_RasterizerState.Get());

        m_InstanceBuffer->Bind(0u);

        uint32_t renderedInstances{ 0u };
        for (const auto& perModel : m_Models)
        {
            perModel.Model->GetVertexBuffer()->Bind(1u);
            perModel.Model->GetIndexBuffer()->Bind();

            for (uint32_t meshIndex{ 0u }; meshIndex < perModel.PerMesh.size(); ++meshIndex)
            {
                const Mesh& srcMesh{ perModel.Model->GetMesh(meshIndex) };
                const auto& meshRange{ perModel.Model->GetMeshRange(meshIndex) };

                for (uint32_t instanceIndex{ 0u }; instanceIndex < srcMesh.GetInstanceCount(); ++instanceIndex)
                {
                    PerDraw perDraw{};
                    perDraw.MeshToModel = srcMesh.GetInstance(instanceIndex);
                    perDraw.ModelToMesh = srcMesh.GetInvInstance(instanceIndex);

                    m_PerDrawConstantBuffer->Set(perDraw);

                    m_PerDrawConstantBuffer->BindVS(2u);

                    uint32_t instancesCount{ static_cast<uint32_t>(perModel.PerMesh[meshIndex].Instances.size()) };
                    DL_THROW_IF_D3D11(deviceContext->DrawIndexedInstanced(meshRange.IndexCount, instancesCount, meshRange.IndexOffset, meshRange.VertexOffset, renderedInstances));
                    renderedInstances += instancesCount;
                }
            }
        }
    }

    void NormalVisGroup::UpdateInstanceBuffer() const
    {
        uint32_t totalInstances{ 0u };
        for (const auto& perModel : m_Models)
            for (const auto& perMesh : perModel.PerMesh)
                totalInstances += static_cast<uint32_t>(perMesh.Instances.size());

        if (totalInstances == 0)
            return;

        m_InstanceBuffer->Resize(totalInstances);

        auto* instanceBufferPtr{ m_InstanceBuffer->Map() };

        uint32_t copiedCount{ 0u };
        for (const auto& perModel : m_Models)
        {
            for (const auto& perMesh : perModel.PerMesh)
            {
                for (const auto& instance : perMesh.Instances)
                {
                    instanceBufferPtr[copiedCount++] = instance;
                }
            }
        }

        m_InstanceBuffer->Unmap();
    }
}
