#pragma once
#include "DLEngine/DirectX/IBindable.h"

struct ShaderSpecification
{
    std::vector<D3D_SHADER_MACRO> Defines;
    std::wstring Path;
    std::string Name;
};

class VertexShader
    : public IBindable
{
public:
    VertexShader(ShaderSpecification spec);
    ~VertexShader() override = default;

    void Bind() override;

    Microsoft::WRL::ComPtr<ID3DBlob> GetVertexShaderBlob() const { return m_VertexShaderBlob; }

private:
    void CompileShader();

private:
    ShaderSpecification m_Specification;

    Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
};

class PixelShader
    : public IBindable
{
public:
    PixelShader(ShaderSpecification spec);
    ~PixelShader() override = default;

    void Bind() override;

private:
    void CompileShader();

private:
    ShaderSpecification m_Specification;

    Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShaderBlob;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
};
