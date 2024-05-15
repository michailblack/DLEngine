#pragma once
#include "DLEngine/DirectX/IBindable.h"
#include "DLEngine/DirectX/Shaders.h"
#include "DLEngine/DirectX/VertexLayout.h"

class InputLayout
    : public IBindable
{
public:
    InputLayout(const VertexLayout& vertexLayout, const Ref<VertexShader>& vertexShader);
    ~InputLayout() override = default;

    void Bind() override;

private:
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
};
