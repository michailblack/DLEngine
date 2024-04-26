#include "dlpch.h"
#include "RenderCommand.h"

#include "DLEngine/Core/D3D.h"

void RenderCommand::Clear(const Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>& renderTargetView, const Math::Vec4& color)
{
    D3D::Get().GetDeviceContext()->ClearRenderTargetView(renderTargetView.Get(), color.data());
}

void RenderCommand::BindRenderTargetView(const Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>& renderTargetView)
{
    auto* renderTargetView0 { static_cast<ID3D11RenderTargetView*>(renderTargetView.Get()) };
    D3D::Get().GetDeviceContext()->OMSetRenderTargets(1, &renderTargetView0, nullptr);
}
