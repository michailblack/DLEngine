#include "WorldLayer.h"

#include "DLEngine/Core/DLException.h"

#include "DLEngine/DirectX/D3D.h"

#define HelloShaderToyPS 1

WorldLayer::WorldLayer()
    : m_CameraController(Camera { Math::ToRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    const auto& device { D3D::Get().GetDevice() };

    ShaderSpecification shaderSpec {};
    shaderSpec.Name = "HelloTriangleVS";
    shaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\HelloTriangleVS.hlsl";

    m_VertexShader = CreateRef<VertexShader>(shaderSpec);

    shaderSpec.Name = "HelloTrianglePS";
    shaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\HelloTrianglePS.hlsl";
#if HelloShaderToyPS
    shaderSpec.Defines.push_back({ "HelloShaderToy", nullptr });
#endif

    m_PixelShader = CreateRef<PixelShader>(shaderSpec);

    struct Vertex
    {
        Math::Vec3 Position;
        Math::Vec3 Color;
    };
    constexpr Vertex vertices[]
    {
        { Math::Vec3 { -0.5f, -0.5f, 0.0f }, Math::Vec3 { 1.0f, 0.0f, 0.0f } },
        { Math::Vec3 { 0.5f, -0.5f, 0.0f }, Math::Vec3 { 0.0f, 1.0f, 0.0f } },
        { Math::Vec3 { 0.0f, 0.5f, 0.0f }, Math::Vec3 { 0.0f, 0.0f, 1.0f } }
    };

    VertexLayout vertexLayout {};
    vertexLayout.Append({ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0u });
    vertexLayout.Append({ "COLOR", DXGI_FORMAT_R32G32B32_FLOAT, 0u });

    m_VertexBuffer = CreateRef<VertexBuffer>(vertexLayout, vertices, 3u);

    m_InputLayout = CreateRef<InputLayout>(vertexLayout, m_VertexShader);

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

    m_ConstantBuffer = CreateRef<PixelConstantBuffer<decltype(m_PerFrameData)>>();
}

void WorldLayer::OnDetach()
{
    
}

void WorldLayer::OnUpdate(float dt)
{
    m_PerFrameData.Time += dt * 1e-3f; // In seconds
    m_PerFrameData.Resolution = Application::Get().GetWindow()->GetSize();

    DrawTestTriangle();
}

void WorldLayer::OnEvent(Event& e)
{
    m_CameraController.OnEvent(e);
}

void WorldLayer::DrawTestTriangle()
{
    const auto& renderTargetView { Application::Get().GetWindow()->GetRenderTargetView() };
    const auto& deviceContext { D3D::Get().GetDeviceContext() };

    auto* renderTargetView0 { static_cast<ID3D11RenderTargetView*>(renderTargetView.Get()) };
    D3D::Get().GetDeviceContext()->OMSetRenderTargets(1, &renderTargetView0, nullptr);

    D3D::Get().GetDeviceContext()->ClearRenderTargetView(renderTargetView.Get(), Math::Vec4 { 0.1f, 0.1f, 0.1f, 1.0f }.data());

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
