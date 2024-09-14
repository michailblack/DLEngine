#include "dlpch.h"
#include "D3D11Context.h"

#include "DLEngine/DirectX/DXGIInfoQueue.h"

#pragma comment(lib, "d3d11.lib")

namespace DLEngine
{
    void D3D11Context::Init()
    {
        UINT createDeviceFlags{ 0 };

#ifdef DL_DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DL_DEBUG

        D3D_FEATURE_LEVEL desiredFeatureLevel{ D3D_FEATURE_LEVEL_11_1 };

        DL_THROW_IF_HR(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            &desiredFeatureLevel, 1u,
            D3D11_SDK_VERSION,
            &m_Device,
            nullptr,
            &m_DeviceContext
        ));

        DL_THROW_IF_HR(CreateDXGIFactory(__uuidof(IDXGIFactory), &m_Factory));
        DL_THROW_IF_HR(m_Factory.As(&m_Factory7));

        DL_THROW_IF_HR(m_Device.As(&m_Device5));
        DL_THROW_IF_HR(m_DeviceContext.As(&m_DeviceContext4));

#ifdef DL_DEBUG
        DL_THROW_IF_HR(m_Device5.As(&m_InfoQueue));
#endif // DL_DEBUG

        DXGIInfoQueue::Get().Init();

        DL_LOG_INFO_TAG("DirectX", "D3D11 Context Initialized");
    }
}
