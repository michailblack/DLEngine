#include "dlpch.h"
#include "Renderer.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/DirectX/ConstantBuffers.h"

#include "DLEngine/Math/Intersections.h"
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
            Ref<ConstantBuffer<PerFrameData>> PerFrameConstantBuffer;

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
            Ref<ConstantBuffer<PerViewData>> PerViewConstantBuffer;

            Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState;
        } s_Data;
    }

    void Renderer::Init()
    {
        const auto& device{ D3D::GetDevice5() };
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        
        s_Data.PerFrameConstantBuffer = CreateRef<ConstantBuffer<RenderData::PerFrameData>>();
        s_Data.PerViewConstantBuffer = CreateRef<ConstantBuffer<RenderData::PerViewData>>();

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = TRUE;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
        depthStencilDesc.StencilEnable = FALSE;

        DL_THROW_IF_HR(device->CreateDepthStencilState(&depthStencilDesc, &s_Data.DepthStencilState));
        deviceContext->OMSetDepthStencilState(s_Data.DepthStencilState.Get(), 1u);
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void Renderer::OnFrameBegin(DeltaTime dt)
    {
        s_Data.PerFrame.TimeMS += dt.GetMilliseconds();
        s_Data.PerFrame.TimeS += dt.GetSeconds();
        s_Data.PerFrame.Resolution = Application::Get().GetWindow()->GetSize();
        s_Data.PerFrame.MousePos = Input::GetCursorPosition();

        s_Data.PerFrameConstantBuffer->Set(s_Data.PerFrame);
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

        s_Data.PerViewConstantBuffer->Set(s_Data.PerView);
    }

    void Renderer::EndScene()
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        const auto& backBufferView{ Application::Get().GetWindow()->GetBackBufferView() };
        const auto& depthStencilView{ Application::Get().GetWindow()->GetDepthStencilView() };

        auto* renderTarget{ static_cast<ID3D11RenderTargetView*>(backBufferView.Get()) };
        deviceContext->OMSetRenderTargets(
            1,
            &renderTarget,
            depthStencilView.Get()
        );

        deviceContext->ClearRenderTargetView(backBufferView.Get(), DLEngine::Math::Vec4{ 0.1f, 0.1f, 0.1f, 1.0f }.data());
        deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 0.0f, 0);
        
        s_Data.PerFrameConstantBuffer->BindVS(0u);
        s_Data.PerFrameConstantBuffer->BindPS(0u);

        s_Data.PerViewConstantBuffer->BindVS(1u);
        s_Data.PerViewConstantBuffer->BindPS(1u);

        MeshSystem::Render();
    }

    void Renderer::Submit(const Ref<Model>& model, const std::any& material, const std::any& instance)
    {
        MeshSystem::AddModel(model, material, instance);
    }
}
