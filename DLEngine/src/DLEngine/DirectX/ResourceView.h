#pragma once
#include "DLEngine/DirectX/D3D.h"

#include "DLEngine/Math/Vec4.h"

namespace DLEngine
{
    class RenderTargetView
    {
    public:
        void Create(const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& texture);
        void Clear(const Math::Vec4& color = Math::Vec4{ 1.0f, 0.0f, 1.0f, 1.0f }) const noexcept;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> GetComPtr() const noexcept { return m_RenderTargetView; }

        void Reset() noexcept { m_RenderTargetView.Reset(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> m_RenderTargetView;
    };

    class DepthStencilView
    {
    public:
        void Create(const Microsoft::WRL::ComPtr<ID3D11Texture2D1>& texture);
        void Clear(float depth = 0.0f, uint8_t stencil = 0) const noexcept;

        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetComPtr() const noexcept { return m_DepthStencilView; }

        void Reset() noexcept { m_DepthStencilView.Reset(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
    };
}

