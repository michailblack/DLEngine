#include "dlpch.h"
#include "Renderer.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"
#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/ConstantBuffer.h"
#include "DLEngine/DirectX/D3DStates.h"
#include "DLEngine/DirectX/SwapChain.h"
#include "DLEngine/DirectX/Texture.h"
#include "DLEngine/DirectX/View.h"

#include "DLEngine/Renderer/Camera.h"
#include "DLEngine/Renderer/PostProcess.h"

#include "DLEngine/Systems/Light/LightSystem.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"

#include "DLEngine/Systems/Transform/TransformSystem.h"

namespace DLEngine
{
    namespace
    {
        struct PerFrameData
        {
            float TimeMS{ 0u };
            float TimeS{ 0u };
            Math::Vec2 Resolution;
            Math::Vec2 MousePos;
            uint8_t _padding[8]{};
        };

        struct PerViewData
        {
            Math::Mat4x4 Projection;
            Math::Mat4x4 InvProjection;
            Math::Mat4x4 View;
            Math::Mat4x4 InvView;
            Math::Mat4x4 ViewProjection;
            Math::Mat4x4 InvViewProjection;
            Math::Vec4 CameraPosition;
            Math::Vec4 BL;
            Math::Vec4 BL2TL;
            Math::Vec4 BL2BR;
        };

        struct
        {
            PerFrameData PerFrame;    
            ConstantBuffer<PerFrameData> PerFrameCB;

            PerViewData PerView;
            ConstantBuffer<PerViewData> PerViewCB;

            PipelineState SkyboxPipelineState;
            ShaderResourceView SkyboxSRV;

            Texture2D FrameTex;
            RenderTargetView FrameView;
            ShaderResourceView FrameSRV;

            PostProcess PostProcess;

            SwapChain SwapChain;
            Texture2D BackBufferTex;
            Texture2D DepthStencilTex;
            RenderTargetView BackBufferView;
            DepthStencilView DepthStencilBufferView;
        } s_Data;
    }

    void Renderer::Init()
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        const auto& window{ Application::Get().GetWindow() };

        s_Data.PerFrameCB.Create();
        s_Data.PerViewCB.Create();

        s_Data.SwapChain.Create(Application::Get().GetWindow()->GetHandle());
        s_Data.BackBufferTex = s_Data.SwapChain.GetBackBuffer();
        s_Data.BackBufferView.Create(s_Data.BackBufferTex);

        const auto& backBufferDesk{ s_Data.BackBufferTex.GetDesc() };

        D3D11_TEXTURE2D_DESC1 depthStencilDesk{};
        depthStencilDesk.Width = backBufferDesk.Width;
        depthStencilDesk.Height = backBufferDesk.Height;
        depthStencilDesk.MipLevels = 1u;
        depthStencilDesk.ArraySize = 1u;
        depthStencilDesk.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesk.SampleDesc.Count = 1u;
        depthStencilDesk.SampleDesc.Quality = 0u;
        depthStencilDesk.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesk.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesk.CPUAccessFlags = 0;
        depthStencilDesk.MiscFlags = 0;
        depthStencilDesk.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        s_Data.DepthStencilTex.Create(depthStencilDesk);
        s_Data.DepthStencilBufferView.Create(s_Data.DepthStencilTex);

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(window->GetWidth());
        viewport.Height = static_cast<float>(window->GetHeight());
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        deviceContext->RSSetViewports(1, &viewport);

        PipelineStateDesc skyboxPipelineState{};
        skyboxPipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        
        ShaderSpecification shaderSpec{};
        shaderSpec.Path = Filesystem::GetShaderDir() + L"Skybox.hlsl";
        
        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);
        skyboxPipelineState.VS = vs;

        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);
        skyboxPipelineState.PS = ps;

        skyboxPipelineState.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);
        skyboxPipelineState.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEPTH_READ_ONLY);

        s_Data.SkyboxPipelineState.Create(skyboxPipelineState);

        D3D11_TEXTURE2D_DESC1 frameTexDesk{};
        frameTexDesk.Width = window->GetWidth();
        frameTexDesk.Height = window->GetHeight();
        frameTexDesk.MipLevels = 1u;
        frameTexDesk.ArraySize = 1u;
        frameTexDesk.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        frameTexDesk.SampleDesc.Count = 1u;
        frameTexDesk.SampleDesc.Quality = 0u;
        frameTexDesk.Usage = D3D11_USAGE_DEFAULT;
        frameTexDesk.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        frameTexDesk.CPUAccessFlags = 0u;
        frameTexDesk.MiscFlags = 0u;
        frameTexDesk.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        s_Data.FrameTex.Create(frameTexDesk);
        s_Data.FrameView.Create(s_Data.FrameTex);
        s_Data.FrameSRV.Create(s_Data.FrameTex);

        s_Data.PostProcess.Create();

        DL_LOG_INFO("Renderer Initialized");
    }

    void Renderer::OnResize(uint32_t width, uint32_t height)
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        deviceContext->RSSetViewports(1, &viewport);

        deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

        s_Data.BackBufferView.Handle.Reset();
        s_Data.DepthStencilBufferView.Handle.Reset();

        s_Data.BackBufferTex.Handle.Reset();
        s_Data.DepthStencilTex.Handle.Reset();

        s_Data.SwapChain.Resize(width, height);

        s_Data.BackBufferTex = s_Data.SwapChain.GetBackBuffer();
        s_Data.BackBufferView.Create(s_Data.BackBufferTex);

        const auto& backBufferDesk{ s_Data.BackBufferTex.GetDesc() };

        D3D11_TEXTURE2D_DESC1 depthStencilDesk{};
        depthStencilDesk.Width = backBufferDesk.Width;
        depthStencilDesk.Height = backBufferDesk.Height;
        depthStencilDesk.MipLevels = 1u;
        depthStencilDesk.ArraySize = 1u;
        depthStencilDesk.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesk.SampleDesc.Count = 1u;
        depthStencilDesk.SampleDesc.Quality = 0u;
        depthStencilDesk.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesk.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesk.CPUAccessFlags = 0u;
        depthStencilDesk.MiscFlags = 0u;
        depthStencilDesk.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        s_Data.DepthStencilTex.Create(depthStencilDesk);
        s_Data.DepthStencilBufferView.Create(s_Data.DepthStencilTex);

        s_Data.FrameTex.Handle.Reset();
        s_Data.FrameView.Handle.Reset();
        s_Data.FrameSRV.Handle.Reset();

        D3D11_TEXTURE2D_DESC1 frameTexDesk{};
        frameTexDesk.Width = width;
        frameTexDesk.Height = height;
        frameTexDesk.MipLevels = 1u;
        frameTexDesk.ArraySize = 1u;
        frameTexDesk.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        frameTexDesk.SampleDesc.Count = 1u;
        frameTexDesk.SampleDesc.Quality = 0u;
        frameTexDesk.Usage = D3D11_USAGE_DEFAULT;
        frameTexDesk.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        frameTexDesk.CPUAccessFlags = 0u;
        frameTexDesk.MiscFlags = 0u;
        frameTexDesk.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        s_Data.FrameTex.Create(frameTexDesk);
        s_Data.FrameView.Create(s_Data.FrameTex);
        s_Data.FrameSRV.Create(s_Data.FrameTex);
    }

    void Renderer::BeginFrame(DeltaTime dt)
    {
        s_Data.PerFrame.TimeMS += dt.GetMilliseconds();
        s_Data.PerFrame.TimeS += dt.GetSeconds();
        s_Data.PerFrame.Resolution = Application::Get().GetWindow()->GetSize();
        s_Data.PerFrame.MousePos = Input::GetCursorPosition();

        s_Data.PerFrameCB.Set(&s_Data.PerFrame, 1u);

        s_Data.PerFrameCB.Bind(0u, BIND_ALL);
        s_Data.PerViewCB.Bind(1u, BIND_ALL);

        D3DStates::GetSamplerState(SamplerStates::POINT_WRAP).Bind(0u, BIND_ALL);
        D3DStates::GetSamplerState(SamplerStates::POINT_CLAMP).Bind(1u, BIND_ALL);

        D3DStates::GetSamplerState(SamplerStates::TRILINEAR_WRAP).Bind(2u, BIND_ALL);
        D3DStates::GetSamplerState(SamplerStates::TRILINEAR_CLAMP).Bind(3u, BIND_ALL);

        D3DStates::GetSamplerState(SamplerStates::ANISOTROPIC_8_WRAP).Bind(4u, BIND_ALL);
        D3DStates::GetSamplerState(SamplerStates::ANISOTROPIC_8_CLAMP).Bind(5u, BIND_ALL);
    }

    void Renderer::EndFrame()
    {
        s_Data.SwapChain.Present();
    }

    void Renderer::BeginScene(const Camera& camera)
    {
        s_Data.PerView.Projection = camera.GetProjectionMatrix();
        s_Data.PerView.InvProjection = Math::Mat4x4::Inverse(s_Data.PerView.Projection);
        s_Data.PerView.View = camera.GetViewMatrix();
        s_Data.PerView.InvView = Math::Mat4x4::Inverse(s_Data.PerView.View);
        s_Data.PerView.ViewProjection = s_Data.PerView.View * s_Data.PerView.Projection;
        s_Data.PerView.InvViewProjection = Math::Mat4x4::Inverse(s_Data.PerView.ViewProjection);
        s_Data.PerView.CameraPosition = Math::Vec4{ camera.GetPosition(), 1.0f };

        const auto& windowSize{ Application::Get().GetWindow()->GetSize() };
        s_Data.PerView.BL = Math::Vec4{ camera.ConstructFrustumPosRotOnly(Math::Vec2{ 0.0f, windowSize.y }), 1.0f };
        s_Data.PerView.BL2TL = Math::Vec4{ camera.ConstructFrustumPosRotOnly(Math::Vec2{ 0.0f, 0.0f }), 1.0f } - s_Data.PerView.BL;
        s_Data.PerView.BL2BR = Math::Vec4{ camera.ConstructFrustumPosRotOnly(Math::Vec2{ windowSize.x, windowSize.y }), 1.0f } - s_Data.PerView.BL;

        s_Data.PerViewCB.Set(&s_Data.PerView, 1u);
    }

    void Renderer::EndScene()
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        auto* renderTarget{ static_cast<ID3D11RenderTargetView*>(s_Data.FrameView.Handle.Get()) };
        deviceContext->OMSetRenderTargets(
            1u,
            &renderTarget,
            s_Data.DepthStencilBufferView.Handle.Get()
        );
        s_Data.FrameView.Clear(Math::Vec4{ 1.0f, 0.0f, 1.0f, 1.0f });
        s_Data.DepthStencilBufferView.Clear(0.0f);

        TransformSystem::Update();
        LightSystem::Update();
        MeshSystem::Get().Render();

        // Drawing skybox
        s_Data.SkyboxPipelineState.Bind();
        s_Data.SkyboxSRV.Bind(0u, BIND_PS);

        DL_THROW_IF_D3D11(deviceContext->Draw(3u, 0u));

        deviceContext->OMSetRenderTargets(0u, nullptr, nullptr);

        s_Data.PostProcess.Resolve(s_Data.FrameSRV, s_Data.BackBufferView);

        deviceContext->OMSetRenderTargets(0u, nullptr, nullptr);
    }

    void Renderer::SetSkybox(const ShaderResourceView& skyboxSRV)
    {
        s_Data.SkyboxSRV = skyboxSRV;
    }

    void Renderer::SetPostProcessSettings(const PostProcessSettings& settings) noexcept
    {
        s_Data.PostProcess.SetSettings(settings);
    }
}
