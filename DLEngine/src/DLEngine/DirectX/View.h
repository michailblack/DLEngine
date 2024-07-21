#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class RenderTargetView
    {
        friend class RenderCommand;
    public:
        void Reset() noexcept { m_Handle.Reset(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> m_Handle{};
    };

    class DepthStencilView
    {
        friend class RenderCommand;
    public:
        void Reset() noexcept { m_Handle.Reset(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_Handle{};
    };

    class ShaderResourceView
    {
        friend class RenderCommand;
    public:
        void Reset() noexcept { m_Handle.Reset(); }

        bool operator==(const ShaderResourceView& other) const noexcept { return m_Handle == other.m_Handle; }

    private:
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> m_Handle{};
    };
}

