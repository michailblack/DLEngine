#pragma once
#include "DLEngine/DirectX/D3D.h"

#include "DLEngine/Math/Vec4.h"

namespace DLEngine
{
    class Texture2D;

    class RenderTargetView
    {
    public:
        void Create(const Texture2D& texture);
        void Clear(const Math::Vec4& color = Math::Vec4{ 1.0f, 0.0f, 1.0f, 1.0f }) const noexcept;

    public:
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> Handle{};
    };

    class DepthStencilView
    {
    public:
        void Create(const Texture2D& texture);
        void Clear(float depth = 0.0f, uint8_t stencil = 0) const noexcept;

    public:
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> Handle{};
    };

    class ShaderResourceView
    {
    public:
        void Create(const Texture2D& texture);

        void Bind(uint32_t slot, uint8_t shaderBindFlags) const noexcept;

    public:
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> Handle{};
    };
}

