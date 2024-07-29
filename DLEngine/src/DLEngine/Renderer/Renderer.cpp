#include "dlpch.h"
#include "Renderer.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"
#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/ConstantBuffer.h"
#include "DLEngine/DirectX/D3DStates.h"
#include "DLEngine/DirectX/SwapChain.h"
#include "DLEngine/DirectX/Texture.h"

#include "DLEngine/Renderer/Camera.h"
#include "DLEngine/Renderer/PostProcess.h"
#include "DLEngine/Renderer/ReflectionCapture.h"

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
            Texture2D SkyboxTex;

            Texture2D FrameTex;

            PostProcess PostProcess;
            ReflectionCapture ReflectionCapture;

            SwapChain SwapChain;
            Texture2D BackBufferTex;
            Texture2D DepthStencilTex;
        } s_Data;
    }

    void Renderer::Init()
    {
        const auto& window{ Application::Get().GetWindow() };

        s_Data.PerFrameCB.Create(sizeof(PerFrameData));
        s_Data.PerViewCB.Create(sizeof(PerViewData));

        s_Data.SwapChain.Create(Application::Get().GetWindow()->GetHandle());

        OnResize(window->GetWidth(), window->GetHeight());

        InitSkyboxPipeline();

        s_Data.PostProcess.Create();
        s_Data.ReflectionCapture.Create(4096u);

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

        RenderCommand::SetRenderTargets();

        s_Data.BackBufferTex.Reset();
        s_Data.DepthStencilTex.Reset();

        s_Data.SwapChain.Resize(width, height);

        s_Data.BackBufferTex = RenderCommand::GetBackBuffer(s_Data.SwapChain);

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

        s_Data.FrameTex.Reset();

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

        RenderCommand::SetShaderResources(
            5u,
            ShaderStage::Pixel,
            {
                s_Data.ReflectionCapture.GetDiffuseIrradiance().GetSRV(),
                s_Data.ReflectionCapture.GetSpecularIrradiance().GetSRV(),
                s_Data.ReflectionCapture.GetSpecularFactor().GetSRV()
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
            { s_Data.FrameTex.GetRTV() },
            s_Data.DepthStencilTex.GetDSV()
        );

        RenderCommand::ClearRenderTargetView(s_Data.FrameTex.GetRTV());
        RenderCommand::ClearDepthStencilView(s_Data.DepthStencilTex.GetDSV());

        TransformSystem::Update();
        LightSystem::Update();
        MeshSystem::Get().Render();

        DrawSkybox();

        RenderCommand::SetRenderTargets();

        s_Data.PostProcess.Resolve(s_Data.FrameTex, s_Data.BackBufferTex);

        RenderCommand::SetRenderTargets();
    }

    void Renderer::SetSkybox(const Texture2D& skybox)
    {
        s_Data.SkyboxTex = skybox;

        s_Data.ReflectionCapture.Build(s_Data.SkyboxTex);
    }

    void Renderer::SetPostProcessSettings(const PostProcessSettings& settings) noexcept
    {
        s_Data.PostProcess.SetSettings(settings);
    }

    void Renderer::InitSkyboxPipeline() noexcept
    {
        s_Data.SkyboxPipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        ShaderSpecification shaderSpec{};
        shaderSpec.Path = Filesystem::GetShaderDir() + L"Skybox.hlsl";

        shaderSpec.EntryPoint = "mainVS";
        s_Data.SkyboxPipelineState.VS.Create(shaderSpec);

        shaderSpec.EntryPoint = "mainPS";
        s_Data.SkyboxPipelineState.PS.Create(shaderSpec);

        s_Data.SkyboxPipelineState.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);
        s_Data.SkyboxPipelineState.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEPTH_READ_ONLY);
    }

    void Renderer::DrawSkybox() noexcept
    {
        RenderCommand::SetPipelineState(s_Data.SkyboxPipelineState);
        RenderCommand::SetShaderResources(0u, ShaderStage::Pixel, { s_Data.SkyboxTex.GetSRV() });

        RenderCommand::Draw(3u);
    }
}
