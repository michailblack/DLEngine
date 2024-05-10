#pragma once
#include "DLEngine/Core/DLWin.h"
#include <d3d11_4.h>
#include <wrl.h>

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

        Microsoft::WRL::ComPtr<ID3DBlob> GetVertexShaderBlob() const { return m_VertexShaderBlob; }

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

    private:
        void CompileShader();

    private:
        ShaderSpecification m_Specification;

        Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShaderBlob;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
    };
}
