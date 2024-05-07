#pragma once
#include "DLEngine/Core/DLWin.h"

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <d3d11sdklayers.h>

namespace DLEngine
{
    class D3D
    {
    public:
        static void Init();

        static Microsoft::WRL::ComPtr<IDXGIFactory> GetFactory() noexcept;
        static Microsoft::WRL::ComPtr<IDXGIFactory7> GetFactory7() noexcept;
        static Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() noexcept;
        static Microsoft::WRL::ComPtr<ID3D11Device5> GetDevice5() noexcept;
        static Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetDeviceContext() noexcept;
        static Microsoft::WRL::ComPtr<ID3D11DeviceContext4> GetDeviceContext4() noexcept;

        static Microsoft::WRL::ComPtr<ID3D11InfoQueue> GetInfoQueue() noexcept;
    };
}
