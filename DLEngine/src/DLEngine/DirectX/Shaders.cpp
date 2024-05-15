#include "dlpch.h"
#include "Shaders.h"

#include "DLEngine/DirectX/D3D.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace Utils
{
    namespace
    {
        std::string ReadFile(const wchar_t* path)
        {
            std::ifstream shaderFile(path, std::ios::in | std::ios::binary);
            std::string shaderSrc {};

            if (shaderFile.is_open())
            {
                shaderFile.seekg(0, std::ios::end);
                const auto size { shaderFile.tellg() };

                if (size != -1)
                {
                    shaderSrc.resize(size);
                    shaderFile.seekg(0, std::ios::beg);
                    shaderFile.read(shaderSrc.data(), size);
                }
            }

            return shaderSrc;
        }
    }
}

VertexShader::VertexShader(ShaderSpecification spec)
    : m_Specification(std::move(spec))
{
    CompileShader();
}

void VertexShader::Bind()
{
    D3D::Get().GetDeviceContext()->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
}

void VertexShader::CompileShader()
{
    const auto shaderSrc { Utils::ReadFile(m_Specification.Path.data()) };

    std::vector defines { m_Specification.Defines };
    defines.push_back({ nullptr, nullptr });

    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0u;

#ifdef DL_DEBUG
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    DL_THROW_IF_HR(D3DCompile(
        shaderSrc.data(), shaderSrc.size(),
        nullptr, defines.data(), nullptr, "main", "vs_5_0",
        compileFlags, 0,
        &m_VertexShaderBlob, &errorBlob
    ));

    DL_THROW_IF_HR(D3D::Get().GetDevice()->CreateVertexShader(
        m_VertexShaderBlob->GetBufferPointer(),
        m_VertexShaderBlob->GetBufferSize(),
        nullptr,
        &m_VertexShader
    ));
}

PixelShader::PixelShader(ShaderSpecification spec)
    : m_Specification(std::move(spec))
{
    CompileShader();
}

void PixelShader::Bind()
{
    D3D::Get().GetDeviceContext()->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
}

void PixelShader::CompileShader()
{
    const auto shaderSrc { Utils::ReadFile(m_Specification.Path.data()) };

    std::vector defines { m_Specification.Defines };
    defines.push_back({ nullptr, nullptr });

    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0u;

#ifdef DL_DEBUG
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    DL_THROW_IF_HR(D3DCompile(
        shaderSrc.data(), shaderSrc.size(),
        nullptr, defines.data(), nullptr, "main", "ps_5_0",
        compileFlags, 0,
        &m_PixelShaderBlob, &errorBlob
    ));

    DL_THROW_IF_HR(D3D::Get().GetDevice()->CreatePixelShader(
        m_PixelShaderBlob->GetBufferPointer(),
        m_PixelShaderBlob->GetBufferSize(),
        nullptr,
        &m_PixelShader
    ));
}
