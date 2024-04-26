#include "dlpch.h"
#include "D3D.h"

#pragma comment(lib, "d3d11.lib")

void D3D::Init()
{
    UINT createDeviceFlags { 0 };

#ifdef DL_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

    DL_THROW_IF(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &device,
        nullptr,
        &deviceContext
    ));

    DL_THROW_IF(device.As(&m_Device));
    DL_THROW_IF(deviceContext.As(&m_DeviceContext));
}
