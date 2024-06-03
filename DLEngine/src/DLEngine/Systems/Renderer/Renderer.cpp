#include "dlpch.h"
#include "Renderer.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/DirectX/ConstantBuffer.h"
#include "DLEngine/DirectX/DXStates.h"
#include "DLEngine/DirectX/SwapChain.h"
#include "DLEngine/DirectX/Texture2D.h"
#include "DLEngine/DirectX/ResourceView.h"

#include "DLEngine/Math/Math.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"

#include "DLEngine/Systems/Renderer/Camera.h"

namespace DLEngine
{
    namespace
    {
        struct
        {
            struct PerFrameData
            {
                float TimeMS{ 0u };
                float TimeS{ 0u };
                Math::Vec2 Resolution;
                Math::Vec2 MousePos;
                uint8_t _padding[8]{};
            } PerFrame;
            ConstantBuffer<PerFrameData> PerFrameCB;

            struct PerViewData
            {
                Math::Mat4x4 Projection;
                Math::Mat4x4 InvProjection;
                Math::Mat4x4 View;
                Math::Mat4x4 InvView;
                Math::Mat4x4 ViewProjection;
                Math::Mat4x4 InvViewProjection;
                Math::Vec4 CameraPosition;
            } PerView;
            ConstantBuffer<PerViewData> PerViewCB;

            SwapChain SwapChain;
            Texture2D DepthStencilTex;
            RenderTargetView BackBufferView;
            DepthStencilView DepthStencilBufferView;
        } s_RendererData;
    }

    void Renderer::Init()
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        const auto& window{ Application::Get().GetWindow() };

        s_RendererData.PerFrameCB.Create();
        s_RendererData.PerViewCB.Create();

        s_RendererData.SwapChain.Create(Application::Get().GetWindow()->GetHandle());

        s_RendererData.BackBufferView.Create(s_RendererData.SwapChain.GetBackBuffer().GetComPtr());

        const auto& backBufferDesk{ s_RendererData.SwapChain.GetBackBuffer().GetDesc() };

        D3D11_TEXTURE2D_DESC1 depthStencilDesk{};
        depthStencilDesk.Width = backBufferDesk.Width;
        depthStencilDesk.Height = backBufferDesk.Height;
        depthStencilDesk.MipLevels = 1;
        depthStencilDesk.ArraySize = 1;
        depthStencilDesk.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesk.SampleDesc.Count = 1;
        depthStencilDesk.SampleDesc.Quality = 0;
        depthStencilDesk.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesk.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesk.CPUAccessFlags = 0;
        depthStencilDesk.MiscFlags = 0;
        depthStencilDesk.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        s_RendererData.DepthStencilTex.Create(depthStencilDesk);

        s_RendererData.DepthStencilBufferView.Create(s_RendererData.DepthStencilTex.GetComPtr());

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(window->GetWidth());
        viewport.Height = static_cast<float>(window->GetHeight());
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        deviceContext->RSSetViewports(1, &viewport);

        DL_LOG_INFO("Renderer Initialized");
    }

    void Renderer::Present()
    {
        s_RendererData.SwapChain.Present();
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

        s_RendererData.BackBufferView.Reset();
        s_RendererData.DepthStencilBufferView.Reset();

        s_RendererData.DepthStencilTex.Reset();

        s_RendererData.SwapChain.Resize(width, height);

        s_RendererData.BackBufferView.Create(s_RendererData.SwapChain.GetBackBuffer().GetComPtr());

        const auto& backBufferDesk{ s_RendererData.SwapChain.GetBackBuffer().GetDesc() };

        D3D11_TEXTURE2D_DESC1 depthStencilDesk{};
        depthStencilDesk.Width = backBufferDesk.Width;
        depthStencilDesk.Height = backBufferDesk.Height;
        depthStencilDesk.MipLevels = 1;
        depthStencilDesk.ArraySize = 1;
        depthStencilDesk.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesk.SampleDesc.Count = 1;
        depthStencilDesk.SampleDesc.Quality = 0;
        depthStencilDesk.Usage = D3D11_USAGE_DEFAULT;
        depthStencilDesk.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesk.CPUAccessFlags = 0;
        depthStencilDesk.MiscFlags = 0;
        depthStencilDesk.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        s_RendererData.DepthStencilTex.Create(depthStencilDesk);

        s_RendererData.DepthStencilBufferView.Create(s_RendererData.DepthStencilTex.GetComPtr());
    }

    void Renderer::OnFrameBegin(DeltaTime dt)
    {
        s_RendererData.PerFrame.TimeMS += dt.GetMilliseconds();
        s_RendererData.PerFrame.TimeS += dt.GetSeconds();
        s_RendererData.PerFrame.Resolution = Application::Get().GetWindow()->GetSize();
        s_RendererData.PerFrame.MousePos = Input::GetCursorPosition();

        s_RendererData.PerFrameCB.Set(s_RendererData.PerFrame);
    }

    void Renderer::BeginScene(const Camera& camera)
    {
        s_RendererData.PerView.Projection = camera.GetProjectionMatrix();
        s_RendererData.PerView.InvProjection = Math::Mat4x4::Inverse(s_RendererData.PerView.Projection);
        s_RendererData.PerView.View = camera.GetViewMatrix();
        s_RendererData.PerView.InvView = Math::Mat4x4::Inverse(s_RendererData.PerView.View);
        s_RendererData.PerView.ViewProjection = s_RendererData.PerView.View * s_RendererData.PerView.Projection;
        s_RendererData.PerView.InvViewProjection = Math::Mat4x4::Inverse(s_RendererData.PerView.ViewProjection);
        s_RendererData.PerView.CameraPosition = Math::Vec4{ camera.GetPosition(), 1.0f };

        s_RendererData.PerViewCB.Set(s_RendererData.PerView);
    }

    void Renderer::EndScene()
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        auto* renderTarget{ static_cast<ID3D11RenderTargetView*>(s_RendererData.BackBufferView.GetComPtr().Get()) };
        deviceContext->OMSetRenderTargets(
            1,
            &renderTarget,
            s_RendererData.DepthStencilBufferView.GetComPtr().Get()
        );
        s_RendererData.BackBufferView.Clear(Math::Vec4{ 0.5f, 0.5f, 0.5f, 1.0f });
        s_RendererData.DepthStencilBufferView.Clear(0.0f);
        
        s_RendererData.PerFrameCB.Bind(0u, CB_BIND_ALL);
        s_RendererData.PerViewCB.Bind(1u, CB_BIND_ALL);

        MeshSystem::Get().Render();
    }
}
