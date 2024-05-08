#include "WorldLayer.h"

#include "DLEngine/Core/DLException.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/VertexBuffer.h"

#define HelloShaderToyPS 1

WorldLayer::WorldLayer()
    : m_CameraController(DLEngine::Camera { DLEngine::Math::ToRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    const auto& device { DLEngine::D3D::GetDevice5() };

    m_Cube = CreateRef<DLEngine::Model>(R"(..\models\cube\cube.obj)");

    DLEngine::ShaderSpecification shaderSpec {};
    shaderSpec.Name = "HelloTriangleVS";
    shaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\HelloTriangleVS.hlsl";

    m_VertexShader = CreateRef<DLEngine::VertexShader>(shaderSpec);

    shaderSpec.Name = "HelloTrianglePS";
    shaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\HelloTrianglePS.hlsl";
#if HelloShaderToyPS
    shaderSpec.Defines.push_back({ "HelloShaderToy", nullptr });
#endif

    m_PixelShader = CreateRef<DLEngine::PixelShader>(shaderSpec);

    const std::vector<Vertex> vertices{
        { DLEngine::Math::Vec3 { -0.5f, -0.5f, 0.0f }, DLEngine::Math::Vec3 { 1.0f, 0.0f, 0.0f } },
        { DLEngine::Math::Vec3 {  0.5f, -0.5f, 0.0f }, DLEngine::Math::Vec3 { 0.0f, 1.0f, 0.0f } },
        { DLEngine::Math::Vec3 {  0.0f,  0.5f, 0.0f }, DLEngine::Math::Vec3 { 0.0f, 0.0f, 1.0f } },
    };

    DLEngine::BufferLayout bufferLayout{
        { "POSITION" , DLEngine::BufferLayout::ShaderDataType::Float3 },
        { "COLOR"    , DLEngine::BufferLayout::ShaderDataType::Float3 },
    };

    m_VertexBuffer = CreateRef<DLEngine::PerVertexBuffer<Vertex>>(bufferLayout, vertices);

    m_InputLayout = CreateRef<DLEngine::InputLayout>();
    m_InputLayout->AppendVertexBuffer(bufferLayout, D3D11_INPUT_PER_VERTEX_DATA);
    m_InputLayout->Construct(m_VertexShader);

    D3D11_RASTERIZER_DESC2 rasterizerDesc {};
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

    m_ConstantBuffer = CreateRef<DLEngine::PixelConstantBuffer<decltype(m_PerFrameData)>>();
}

void WorldLayer::OnDetach()
{
    
}

void WorldLayer::OnUpdate(DeltaTime dt)
{
    m_PerFrameData.Time += dt.GetSeconds();
    m_PerFrameData.Resolution = DLEngine::Application::Get().GetWindow()->GetSize();

    DrawTestTriangle();
}

void WorldLayer::OnEvent(DLEngine::Event& e)
{
    m_CameraController.OnEvent(e);
}

void WorldLayer::DrawTestTriangle()
{
    const auto& deviceContext{ DLEngine::D3D::GetDeviceContext4() };
    
    const auto& backBufferView{ DLEngine::Application::Get().GetWindow()->GetBackBufferView() };
    const auto& depthStencilView { DLEngine::Application::Get().GetWindow()->GetDepthStencilView() };

    auto* renderTarget{ static_cast<ID3D11RenderTargetView*>(backBufferView.Get()) };
    DLEngine::D3D::GetDeviceContext4()->OMSetRenderTargets(
        1,
        &renderTarget,
        depthStencilView.Get()
    );

    DLEngine::D3D::GetDeviceContext4()->ClearRenderTargetView(backBufferView.Get(), DLEngine::Math::Vec4 { 0.1f, 0.1f, 0.1f, 1.0f }.data());
    DLEngine::D3D::GetDeviceContext4()->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_InputLayout->Bind();
    m_VertexBuffer->Bind();

    m_VertexShader->Bind();
    m_PixelShader->Bind();

    m_ConstantBuffer->Set(m_PerFrameData);
    m_ConstantBuffer->Bind();

    deviceContext->RSSetState(m_RasterizerState.Get());

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DL_THROW_IF_D3D11(deviceContext->Draw(3u, 0u));
}
