#pragma once
#include "DLEngine/Core/DLWin.h"
#include <d3d11_4.h>
#include <wrl.h>

#include "DLEngine/Math/Vec4.h"

class RenderCommand
{
public:
    static void Clear(const Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>& renderTargetView, const Math::Vec4& color);
    static void BindRenderTargetView(const Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>& renderTargetView);
};
