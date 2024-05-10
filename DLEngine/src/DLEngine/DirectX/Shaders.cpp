#include "dlpch.h"
#include "Shaders.h"

#include "DLEngine/DirectX/D3D.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            std::string ReadFile(const wchar_t* path)
            {
                std::ifstream shaderFile(path, std::ios::in | std::ios::binary);
                std::string shaderSrc{};

                if (shaderFile.is_open())
                {
                    shaderFile.seekg(0, std::ios::end);
                    const auto size{ shaderFile.tellg() };

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
        D3D::GetDeviceContext4()->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
    }

    void VertexShader::CompileShader()
    {
        const auto shaderSrc{ Utils::ReadFile(m_Specification.Path.data()) };

        std::vector defines{ m_Specification.Defines };
        defines.push_back({ nullptr, nullptr });

        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        UINT compileFlags = 0u;

#ifdef DL_DEBUG
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        if (FAILED(D3DCompile(
            shaderSrc.data(), shaderSrc.size(),
            nullptr, defines.data(), nullptr, "main", "vs_5_0",
            compileFlags, 0,
            &m_VertexShaderBlob, &errorBlob
        )))
        {
            throw std::runtime_error{ static_cast<const char*>(errorBlob->GetBufferPointer()) };
        }

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateVertexShader(
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
        D3D::GetDeviceContext4()->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
    }

    void PixelShader::CompileShader()
    {
        const auto shaderSrc{ Utils::ReadFile(m_Specification.Path.data()) };

        std::vector defines{ m_Specification.Defines };
        defines.push_back({ nullptr, nullptr });

        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        UINT compileFlags = 0u;

#ifdef DL_DEBUG
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        if (FAILED(D3DCompile(
            shaderSrc.data(), shaderSrc.size(),
            nullptr, defines.data(), nullptr, "main", "ps_5_0",
            compileFlags, 0,
            &m_PixelShaderBlob, &errorBlob
        )))
        {
            throw std::runtime_error{ static_cast<const char*>(errorBlob->GetBufferPointer()) };
        }

        DL_THROW_IF_HR(D3D::GetDevice5()->CreatePixelShader(
            m_PixelShaderBlob->GetBufferPointer(),
            m_PixelShaderBlob->GetBufferSize(),
            nullptr,
            &m_PixelShader
        ));
    }
}
