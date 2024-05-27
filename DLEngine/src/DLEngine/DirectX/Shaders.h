#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    struct ShaderSpecification
    {
        std::vector<D3D_SHADER_MACRO> Defines;
        std::string Path;
        std::string Name;
    };

    class VertexShader
    {
    public:
        VertexShader(ShaderSpecification spec);

        void Bind();

        Microsoft::WRL::ComPtr<ID3DBlob> GetBlob() const { return m_VertexShaderBlob; }

        Microsoft::WRL::ComPtr<ID3D11VertexShader> GetComPtr() const { return m_VertexShader; }

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

        Microsoft::WRL::ComPtr<ID3D11PixelShader> GetComPtr() const { return m_PixelShader; }

    private:
        ShaderSpecification m_Specification;

        Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShaderBlob;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
    };

    class HullShader
    {
    public:
        HullShader(ShaderSpecification spec);

        void Bind();

        Microsoft::WRL::ComPtr<ID3D11HullShader> GetComPtr() const { return m_HullShader; }

    private:
        ShaderSpecification m_Specification;

        Microsoft::WRL::ComPtr<ID3DBlob> m_HullShaderBlob;
        Microsoft::WRL::ComPtr<ID3D11HullShader> m_HullShader;
    };

    class DomainShader
    {
    public:
        DomainShader(ShaderSpecification spec);

        void Bind();

        Microsoft::WRL::ComPtr<ID3D11DomainShader> GetComPtr() const { return m_DomainShader; }

    private:
        ShaderSpecification m_Specification;

        Microsoft::WRL::ComPtr<ID3DBlob> m_DomainShaderBlob;
        Microsoft::WRL::ComPtr<ID3D11DomainShader> m_DomainShader;
    };
}
