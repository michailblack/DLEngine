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

    enum ShaderBindType : uint8_t
    {
        BIND_VS = BIT(0),
        BIND_PS = BIT(1),
        BIND_DS = BIT(2),
        BIND_HS = BIT(3),
        BIND_GS = BIT(4),

        BIND_ALL = BIND_VS |
        BIND_PS |
        BIND_DS |
        BIND_HS |
        BIND_GS
    };
}
