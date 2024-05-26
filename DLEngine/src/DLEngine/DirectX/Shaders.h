#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    struct ShaderSpecification
    {
        std::vector<D3D_SHADER_MACRO> Defines;
        std::wstring Path;
        std::string Name;
    };

    class VertexShader
    {
    public:
        VertexShader(ShaderSpecification spec);

        void Bind();

        Microsoft::WRL::ComPtr<ID3DBlob> GetVertexShaderBlob() const noexcept { return m_VertexShaderBlob; }
        Microsoft::WRL::ComPtr<ID3D11VertexShader> GetComPtr() const noexcept { return m_VertexShader; }

    private:
        void CompileShader();

    private:
        ShaderSpecification m_Specification;

        Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShaderBlob;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
    };

    class PixelShader
    {
    public:
        PixelShader(ShaderSpecification spec);

        void Bind();
        Microsoft::WRL::ComPtr<ID3D11PixelShader> GetComPtr() const noexcept { return m_PixelShader; }

    private:
        void CompileShader();

    private:
        ShaderSpecification m_Specification;

        Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShaderBlob;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
    };
}
