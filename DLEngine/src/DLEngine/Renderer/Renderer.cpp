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
            Math::Vec2 Resolution;
            Math::Vec2 MousePos;
            float TimeMS{ 0u };
            float TimeS{ 0u };
            float _padding[2]{};
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
            ConstantBuffer PerFrameCB;

            PerViewData PerView;
            ConstantBuffer PerViewCB;

            PipelineState SkyboxPipelineState;
            ShaderResourceView SkyboxSRV;

            RWTexture2D FrameRWTex;

            PostProcess PostProcess;

            SwapChain SwapChain;
            WTexture2D BackBufferTex;
            DTexture2D DepthStencilTex;
        } s_Data;
    }

    void Renderer::Init()
    {
        const auto& window{ Application::Get().GetWindow() };

        s_Data.PerFrameCB.Create(sizeof(PerFrameData));
        s_Data.PerViewCB.Create(sizeof(PerViewData));

        s_Data.SwapChain.Create(Application::Get().GetWindow()->GetHandle());
        s_Data.BackBufferTex.Create(RenderCommand::GetBackBuffer(s_Data.SwapChain));

        const auto& backBufferDesk{ s_Data.BackBufferTex.GetTexture().GetDesc()};

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

        Texture2D depthStencilTex{};
        depthStencilTex.Create(depthStencilDesk);

        s_Data.DepthStencilTex.Create(depthStencilTex);

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(window->GetWidth());
        viewport.Height = static_cast<float>(window->GetHeight());
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        RenderCommand::SetViewports({ viewport });

        s_Data.SkyboxPipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        
        ShaderSpecification shaderSpec{};
        shaderSpec.Path = Filesystem::GetShaderDir() + L"Skybox.hlsl";
        
        shaderSpec.EntryPoint = "mainVS";
        VertexShader vs{};
        vs.Create(shaderSpec);
        s_Data.SkyboxPipelineState.VS = vs;

        shaderSpec.EntryPoint = "mainPS";
        PixelShader ps{};
        ps.Create(shaderSpec);
        s_Data.SkyboxPipelineState.PS = ps;

        s_Data.SkyboxPipelineState.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);
        s_Data.SkyboxPipelineState.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEPTH_READ_ONLY);

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

        Texture2D frameTex{};
        frameTex.Create(frameTexDesk);

        s_Data.FrameRWTex.Create(frameTex);

        s_Data.PostProcess.Create();

        DL_LOG_INFO("Renderer Initialized");
    }

    void Renderer::OnResize(uint32_t width, uint32_t height)
    {
        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        RenderCommand::SetViewports({ viewport });

        RenderCommand::SetRenderTargets({ RenderTargetView{} }, DepthStencilView{});

        s_Data.BackBufferTex.Reset();
        s_Data.DepthStencilTex.Reset();

        s_Data.SwapChain.Resize(width, height);

        s_Data.BackBufferTex.Create(RenderCommand::GetBackBuffer(s_Data.SwapChain));

        const auto& backBufferDesk{ s_Data.BackBufferTex.GetTexture().GetDesc() };

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

        Texture2D depthStencilTex{};
        depthStencilTex.Create(depthStencilDesk);

        s_Data.DepthStencilTex.Create(depthStencilTex);

        s_Data.FrameRWTex.Reset();

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

        Texture2D frameTex{};
        frameTex.Create(frameTexDesk);

        s_Data.FrameRWTex.Create(frameTex);
    }

    void Renderer::BeginFrame(DeltaTime dt)
    {
        s_Data.PerFrame.TimeMS += dt.GetMilliseconds();
        s_Data.PerFrame.TimeS += dt.GetSeconds();
        s_Data.PerFrame.Resolution = Application::Get().GetWindow()->GetSize();
        s_Data.PerFrame.MousePos = Input::GetCursorPosition();

        s_Data.PerFrameCB.Set(&s_Data.PerFrame);

        RenderCommand::SetConstantBuffers(
            0u,
            ShaderStage::All,
            { s_Data.PerFrameCB, s_Data.PerViewCB }
        );

        RenderCommand::SetSamplers(
            0u,
            ShaderStage::All,
            {
                D3DStates::GetSamplerState(SamplerStates::ANISOTROPIC_8_WRAP),
                D3DStates::GetSamplerState(SamplerStates::POINT_WRAP),
                D3DStates::GetSamplerState(SamplerStates::POINT_CLAMP),
                D3DStates::GetSamplerState(SamplerStates::TRILINEAR_WRAP),
                D3DStates::GetSamplerState(SamplerStates::TRILINEAR_CLAMP),
                D3DStates::GetSamplerState(SamplerStates::ANISOTROPIC_8_WRAP),
                D3DStates::GetSamplerState(SamplerStates::ANISOTROPIC_8_CLAMP)
            }
        );
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

        s_Data.PerViewCB.Set(&s_Data.PerView);
    }

    void Renderer::EndScene()
    {
        RenderCommand::SetRenderTargets(
            { s_Data.FrameRWTex.GetRTV() },
            s_Data.DepthStencilTex.GetDSV()
        );

        RenderCommand::ClearRenderTargetView(s_Data.FrameRWTex.GetRTV());
        RenderCommand::ClearDepthStencilView(s_Data.DepthStencilTex.GetDSV());

        TransformSystem::Update();
        LightSystem::Update();
        MeshSystem::Get().Render();

        // Drawing skybox
        RenderCommand::SetPipelineState(s_Data.SkyboxPipelineState);
        RenderCommand::SetShaderResources(0u, ShaderStage::Pixel, { s_Data.SkyboxSRV });

        RenderCommand::Draw(3u);

        RenderCommand::SetRenderTargets({ RenderTargetView{} }, DepthStencilView{});

        s_Data.PostProcess.Resolve(s_Data.FrameRWTex.GetSRV(), s_Data.BackBufferTex.GetRTV());

        RenderCommand::SetRenderTargets({ RenderTargetView{} }, DepthStencilView{});
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
