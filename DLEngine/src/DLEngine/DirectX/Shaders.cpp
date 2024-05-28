#include "dlpch.h"
#include "Shaders.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            std::string ReadFile(const char* path)
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

            enum class ShaderType
            {
                Vertex,
                Pixel,
                Hull,
                Domain,
            };

            void CompileShader(const ShaderSpecification& spec, ShaderType type, Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob)
            {
                const auto shaderSrc{ Utils::ReadFile(spec.Path.data()) };

                std::vector defines{ spec.Defines };
                defines.push_back({ nullptr, nullptr });

                Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

                UINT compileFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR |
                    D3DCOMPILE_ENABLE_STRICTNESS |
                    D3DCOMPILE_WARNINGS_ARE_ERRORS;

#ifdef DL_DEBUG
                compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
                compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

                const char* target{};
                switch (type)
                {
                case ShaderType::Vertex:
                    target = "vs_5_0";
                    break;
                case ShaderType::Pixel:
                    target = "ps_5_0";
                    break;
                case ShaderType::Hull:
                    target = "hs_5_0";
                    break;
                case ShaderType::Domain:
                    target = "ds_5_0";
                    break;
                }

                if (FAILED(D3DCompile(
                    shaderSrc.data(), shaderSrc.size(),
                    nullptr, defines.data(), nullptr, "main", target,
                    compileFlags, 0,
                    &shaderBlob, &errorBlob
                )))
                {
                    std::ostringstream oss;
                    oss << "Failed to compile shader: " << spec.Name << '\n';
                    oss << "Error: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << '\n';
                    throw std::runtime_error{ oss.str() };
                }

                DL_LOG_INFO("Successfully compiled shader: {}", spec.Name);
                DL_LOG_INFO("\t[FILE]: {}", spec.Path);
            }
        }
    }

    VertexShader::VertexShader(ShaderSpecification spec)
        : m_Specification(std::move(spec))
    {
        Utils::CompileShader(m_Specification, Utils::ShaderType::Vertex, m_VertexShaderBlob);

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateVertexShader(
            m_VertexShaderBlob->GetBufferPointer(),
            m_VertexShaderBlob->GetBufferSize(),
            nullptr,
            &m_VertexShader
        ));
    }

    void VertexShader::Bind()
    {
        D3D::GetDeviceContext4()->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
    }

    PixelShader::PixelShader(ShaderSpecification spec)
        : m_Specification(std::move(spec))
    {
        Utils::CompileShader(m_Specification, Utils::ShaderType::Pixel, m_PixelShaderBlob);

        DL_THROW_IF_HR(D3D::GetDevice5()->CreatePixelShader(
            m_PixelShaderBlob->GetBufferPointer(),
            m_PixelShaderBlob->GetBufferSize(),
            nullptr,
            &m_PixelShader
        ));
    }

    void PixelShader::Bind()
    {
        D3D::GetDeviceContext4()->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
    }

    HullShader::HullShader(ShaderSpecification spec)
        : m_Specification(std::move(spec))
    {
        Utils::CompileShader(m_Specification, Utils::ShaderType::Hull, m_HullShaderBlob);

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateHullShader(
            m_HullShaderBlob->GetBufferPointer(),
            m_HullShaderBlob->GetBufferSize(),
            nullptr,
            &m_HullShader
        ));
    }

    void HullShader::Bind()
    {
        D3D::GetDeviceContext4()->HSSetShader(m_HullShader.Get(), nullptr, 0u);
    }

    DomainShader::DomainShader(ShaderSpecification spec)
        : m_Specification(std::move(spec))
    {
        Utils::CompileShader(m_Specification, Utils::ShaderType::Domain, m_DomainShaderBlob);

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateDomainShader(
            m_DomainShaderBlob->GetBufferPointer(),
            m_DomainShaderBlob->GetBufferSize(),
            nullptr,
            &m_DomainShader
        ));
    }

    void DomainShader::Bind()
    {
        D3D::GetDeviceContext4()->DSSetShader(m_DomainShader.Get(), nullptr, 0u);
    }
}
