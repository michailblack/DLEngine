#include "dlpch.h"
#include "D3D.h"

#pragma comment(lib, "d3d11.lib")

namespace DLEngine
{
    namespace
    {
        struct
        {
            Microsoft::WRL::ComPtr<IDXGIFactory> Factory;
            Microsoft::WRL::ComPtr<IDXGIFactory7> Factory7;
            Microsoft::WRL::ComPtr<ID3D11Device> Device;
            Microsoft::WRL::ComPtr<ID3D11Device5> Device5;
            Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
            Microsoft::WRL::ComPtr<ID3D11DeviceContext4> DeviceContext4;

            Microsoft::WRL::ComPtr<ID3D11InfoQueue> InfoQueue;
        } s_D3DData;
    }

    void D3D::Init()
    {
        UINT createDeviceFlags{ 0 };

#ifdef DL_DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DL_DEBUG

        DL_THROW_IF_HR(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            nullptr, 0,
            D3D11_SDK_VERSION,
            &s_D3DData.Device,
            nullptr,
            &s_D3DData.DeviceContext
        ));

        DL_THROW_IF_HR(CreateDXGIFactory(__uuidof(IDXGIFactory), &s_D3DData.Factory));
        DL_THROW_IF_HR(s_D3DData.Factory.As(&s_D3DData.Factory7));

        DL_THROW_IF_HR(s_D3DData.Device.As(&s_D3DData.Device5));
        DL_THROW_IF_HR(s_D3DData.DeviceContext.As(&s_D3DData.DeviceContext4));

#ifdef DL_DEBUG
        DL_THROW_IF_HR(s_D3DData.Device5.As(&s_D3DData.InfoQueue));
#endif // DL_DEBUG

        DL_LOG_INFO("Direct3D 11 Initialized");
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> D3D::GetFactory() noexcept
    {
        return s_D3DData.Factory;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory7> D3D::GetFactory7() noexcept
    {
        return s_D3DData.Factory7;
    }

    Microsoft::WRL::ComPtr<ID3D11Device> D3D::GetDevice() noexcept
    {
        return s_D3DData.Device;
    }

    Microsoft::WRL::ComPtr<ID3D11Device5> D3D::GetDevice5() noexcept
    {
        return s_D3DData.Device5;
    }

    Microsoft::WRL::ComPtr<ID3D11DeviceContext> D3D::GetDeviceContext() noexcept
    {
        return s_D3DData.DeviceContext;
    }

    Microsoft::WRL::ComPtr<ID3D11DeviceContext4> D3D::GetDeviceContext4() noexcept
    {
        return s_D3DData.DeviceContext4;
    }

    Microsoft::WRL::ComPtr<ID3D11InfoQueue> D3D::GetInfoQueue() noexcept
    {
        return s_D3DData.InfoQueue;
    }
}
