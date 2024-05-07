#include "dlpch.h"
#include "D3D.h"

#pragma comment(lib, "d3d11.lib")

namespace DLEngine
{
    namespace
    {
        struct D3DData
        {
            Microsoft::WRL::ComPtr<IDXGIFactory> Factory;
            Microsoft::WRL::ComPtr<IDXGIFactory7> Factory7;
            Microsoft::WRL::ComPtr<ID3D11Device> Device;
            Microsoft::WRL::ComPtr<ID3D11Device5> Device5;
            Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
            Microsoft::WRL::ComPtr<ID3D11DeviceContext4> DeviceContext4;

            Microsoft::WRL::ComPtr<ID3D11InfoQueue> InfoQueue;
        } s_Data;
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
            &s_Data.Device,
            nullptr,
            &s_Data.DeviceContext
        ));

        DL_THROW_IF_HR(CreateDXGIFactory(__uuidof(IDXGIFactory), &s_Data.Factory));
        DL_THROW_IF_HR(s_Data.Factory.As(&s_Data.Factory7));

        DL_THROW_IF_HR(s_Data.Device.As(&s_Data.Device5));
        DL_THROW_IF_HR(s_Data.DeviceContext.As(&s_Data.DeviceContext4));

#ifdef DL_DEBUG
        DL_THROW_IF_HR(s_Data.Device5.As(&s_Data.InfoQueue));
#endif // DL_DEBUG
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> D3D::GetFactory() noexcept
    {
        return s_Data.Factory;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory7> D3D::GetFactory7() noexcept
    {
        return s_Data.Factory7;
    }

    Microsoft::WRL::ComPtr<ID3D11Device> D3D::GetDevice() noexcept
    {
        return s_Data.Device;
    }

    Microsoft::WRL::ComPtr<ID3D11Device5> D3D::GetDevice5() noexcept
    {
        return s_Data.Device5;
    }

    Microsoft::WRL::ComPtr<ID3D11DeviceContext> D3D::GetDeviceContext() noexcept
    {
        return s_Data.DeviceContext;
    }

    Microsoft::WRL::ComPtr<ID3D11DeviceContext4> D3D::GetDeviceContext4() noexcept
    {
        return s_Data.DeviceContext4;
    }

    Microsoft::WRL::ComPtr<ID3D11InfoQueue> D3D::GetInfoQueue() noexcept
    {
        return s_Data.InfoQueue;
    }
}
