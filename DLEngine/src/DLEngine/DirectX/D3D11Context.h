#pragma once
#include "DLEngine/Core/Application.h"

#include "DLEngine/Renderer/RendererContext.h"

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <d3d11sdklayers.h>

namespace DLEngine
{
    class D3D11Context : public RendererContext
    {
    public:
        void Init() override;

        static Ref<D3D11Context> Get() { return AsRef<D3D11Context>(Application::Get().GetWindow().GetRendererContext()); }

        Microsoft::WRL::ComPtr<IDXGIFactory> GetFactory() noexcept { return m_Factory; }
        Microsoft::WRL::ComPtr<IDXGIFactory7> GetFactory7() noexcept { return m_Factory7; }
        Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() noexcept { return m_Device; }
        Microsoft::WRL::ComPtr<ID3D11Device5> GetDevice5() noexcept { return m_Device5; }
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetDeviceContext() noexcept { return m_DeviceContext; }
        Microsoft::WRL::ComPtr<ID3D11DeviceContext4> GetDeviceContext4() noexcept { return m_DeviceContext4; }

        Microsoft::WRL::ComPtr<ID3D11InfoQueue> GetInfoQueue() noexcept { return m_InfoQueue; }

    private:
        Microsoft::WRL::ComPtr<IDXGIFactory> m_Factory;
        Microsoft::WRL::ComPtr<IDXGIFactory7> m_Factory7;
        Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
        Microsoft::WRL::ComPtr<ID3D11Device5> m_Device5;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext4> m_DeviceContext4;

        Microsoft::WRL::ComPtr<ID3D11InfoQueue> m_InfoQueue;
    };
}

