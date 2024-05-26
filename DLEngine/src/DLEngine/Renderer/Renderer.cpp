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

#include "DLEngine/Mesh/MeshSystem.h"

namespace DLEngine
{
    namespace
    {
        struct RenderData
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
        } s_Data;
    }

    void Renderer::Init()
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        const auto& window{ Application::Get().GetWindow() };

        s_Data.PerFrameCB.Create();
        s_Data.PerViewCB.Create();

        s_Data.SwapChain.Create(Application::Get().GetWindow()->GetHandle());

        s_Data.BackBufferView.Create(s_Data.SwapChain.GetBackBuffer().GetComPtr());

        const auto& backBufferDesk{ s_Data.SwapChain.GetBackBuffer().GetDesc() };

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

        s_Data.DepthStencilTex.Create(depthStencilDesk);

        s_Data.DepthStencilBufferView.Create(s_Data.DepthStencilTex.GetComPtr());

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(window->GetWidth());
        viewport.Height = static_cast<float>(window->GetHeight());
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        deviceContext->RSSetViewports(1, &viewport);
    }

    void Renderer::Present()
    {
        s_Data.SwapChain.Present();
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

        s_Data.BackBufferView.Reset();
        s_Data.DepthStencilBufferView.Reset();

        s_Data.DepthStencilTex.Reset();

        s_Data.SwapChain.Resize(width, height);

        s_Data.BackBufferView.Create(s_Data.SwapChain.GetBackBuffer().GetComPtr());

        const auto& backBufferDesk{ s_Data.SwapChain.GetBackBuffer().GetDesc() };

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

        s_Data.DepthStencilTex.Create(depthStencilDesk);

        s_Data.DepthStencilBufferView.Create(s_Data.DepthStencilTex.GetComPtr());
    }

    void Renderer::OnFrameBegin(DeltaTime dt)
    {
        s_Data.PerFrame.TimeMS += dt.GetMilliseconds();
        s_Data.PerFrame.TimeS += dt.GetSeconds();
        s_Data.PerFrame.Resolution = Application::Get().GetWindow()->GetSize();
        s_Data.PerFrame.MousePos = Input::GetCursorPosition();

        s_Data.PerFrameCB.Set(s_Data.PerFrame);
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

        s_Data.PerViewCB.Set(s_Data.PerView);
    }

    void Renderer::EndScene()
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        auto* renderTarget{ static_cast<ID3D11RenderTargetView*>(s_Data.BackBufferView.GetComPtr().Get()) };
        deviceContext->OMSetRenderTargets(
            1,
            &renderTarget,
            s_Data.DepthStencilBufferView.GetComPtr().Get()
        );
        s_Data.BackBufferView.Clear(Math::Vec4{ 0.1f, 0.1f, 0.1f, 1.0f });
        s_Data.DepthStencilBufferView.Clear(0.0f);
        
        s_Data.PerFrameCB.Bind(0u, CB_BIND_VS | CB_BIND_PS);
        s_Data.PerViewCB.Bind(1u, CB_BIND_VS | CB_BIND_PS);

        MeshSystem::Get().Render();
    }
}
