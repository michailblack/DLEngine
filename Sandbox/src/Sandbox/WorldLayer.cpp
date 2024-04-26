#include "WorldLayer.h"

#include "DLEngine/Core/D3D.h"
#include "DLEngine/Core/DLException.h"
#include "DLEngine/Math/Intersections.h"

#include "DLEngine/Renderer/RenderCommand.h"
#include "DLEngine/Renderer/Renderer.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

#define HelloTrianglePS 1

WorldLayer::WorldLayer()
    : m_CameraController(Camera { Math::ToRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    using namespace Microsoft::WRL;

    const auto& device { D3D::Get().GetDevice() };

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

    D3D11_BUFFER_DESC vertexBufferDesc {};
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3u;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0u;
    vertexBufferDesc.MiscFlags = 0u;
    vertexBufferDesc.StructureByteStride = sizeof(Vertex);

    D3D11_SUBRESOURCE_DATA vertexBufferData {};
    vertexBufferData.pSysMem = vertices;
    vertexBufferData.SysMemPitch = 0u;
    vertexBufferData.SysMemSlicePitch = 0u;

    DL_THROW_IF(device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_VertexBuffer));

    ComPtr<ID3DBlob> errorBlob;

    ComPtr<ID3DBlob> vertexShaderBlob;

    UINT compileFlags = 0u;

#ifdef _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    DL_THROW_IF(D3DCompileFromFile(
        L"..\\DLEngine\\src\\DLEngine\\Shaders\\HelloTriangleVS.hlsl",
        nullptr, nullptr,
        "main", "vs_5_0",
        compileFlags, 0u,
        &vertexShaderBlob, &errorBlob
    ));

    DL_THROW_IF(device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(),
        nullptr,
        &m_VertexShader
    ));

    ComPtr<ID3DBlob> pixelShaderBlob;

#if HelloTrianglePS
    D3D_SHADER_MACRO shaderMacros[]
    {
        { nullptr, nullptr }
    };
#else
    D3D_SHADER_MACRO shaderMacros[]
    {
        { "HelloShaderToy", "1" },
        { nullptr, nullptr }
    };
#endif

    DL_THROW_IF(D3DCompileFromFile(
        L"..\\DLEngine\\src\\DLEngine\\Shaders\\HelloTrianglePS.hlsl",
        shaderMacros, nullptr,
        "main", "ps_5_0",
        compileFlags, 0u,
        &pixelShaderBlob, &errorBlob
    ));

    DL_THROW_IF(device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(),
        nullptr,
        &m_PixelShader
    ));

    D3D11_INPUT_ELEMENT_DESC inputElementDescs[]
    {
        { "POSITION", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
        { "COLOR", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 12u, D3D11_INPUT_PER_VERTEX_DATA, 0u }
    };

    DL_THROW_IF(device->CreateInputLayout(
        inputElementDescs, 2,
        vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(),
        &m_InputLayout
    ));

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

    DL_THROW_IF(device->CreateRasterizerState2(&rasterizerDesc, &m_RasterizerState));

    D3D11_BUFFER_DESC constantBufferDesc {};
    constantBufferDesc.ByteWidth = sizeof(m_PerFrameData);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags = 0u;
    constantBufferDesc.StructureByteStride = 0u;

    DL_THROW_IF(device->CreateBuffer(&constantBufferDesc, nullptr, &m_ConstantBuffer));
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

    RenderCommand::BindRenderTargetView(renderTargetView);
    RenderCommand::Clear(renderTargetView, Math::Vec4 { 0.1f, 0.1f, 0.1f, 1.0f });

    D3D11_MAPPED_SUBRESOURCE mappedSubresource {};
    deviceContext->Map(m_ConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource);

    memcpy_s(mappedSubresource.pData, sizeof(m_PerFrameData), &m_PerFrameData, sizeof(m_PerFrameData));

    deviceContext->Unmap(m_ConstantBuffer.Get(), 0u);

    deviceContext->PSSetConstantBuffers1(0u, 1u, m_ConstantBuffer.GetAddressOf(), nullptr, nullptr);

    deviceContext->RSSetState(m_RasterizerState.Get());

    deviceContext->IASetInputLayout(m_InputLayout.Get());

    deviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
    deviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);

    constexpr uint32_t stride { 24u };
    constexpr uint32_t offset { 0u };
    deviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->Draw(3u, 0u);
}
