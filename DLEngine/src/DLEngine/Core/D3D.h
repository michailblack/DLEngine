#pragma once
#include "DLEngine/Core/DLWin.h"

#include <d3d11_4.h>
#include <wrl.h>

class D3D
{
public:
    static D3D& Get() noexcept
    {
        static D3D s_Instance;
        return s_Instance;
    }

    void Init();

    Microsoft::WRL::ComPtr<ID3D11Device5> GetDevice() const noexcept { return m_Device; }
    Microsoft::WRL::ComPtr<ID3D11DeviceContext4> GetDeviceContext() const noexcept { return m_DeviceContext; }

private:
    D3D() = default;

private:
    Microsoft::WRL::ComPtr<ID3D11Device5> m_Device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext4> m_DeviceContext;
};
