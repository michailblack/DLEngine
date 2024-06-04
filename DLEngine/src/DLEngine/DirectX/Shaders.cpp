#include "dlpch.h"
#include "Shaders.h"

#include "DLEngine/Core/Filesystem.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            std::vector<char> ReadFile(const wchar_t* path)
            {
                std::ifstream shaderFile(path, std::ios::in | std::ios::binary | std::ios::ate);
                std::vector<char> shaderSrc{};

                if (shaderFile.is_open())
                {
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
                Geometry
            };

            void CompileShader(const ShaderSpecification& spec, ShaderType type, Microsoft::WRL::ComPtr<ID3DBlob>& shaderBlob)
            {
                const auto shaderSrc{ Utils::ReadFile(spec.Path.data()) };
                
                uint32_t shaderSrcNameStart{ static_cast<uint32_t>(spec.Path.find_last_of('\\')) };
                std::wstring_view shaderSrcName{
                    spec.Path.data() + shaderSrcNameStart + 1u,
                    spec.Path.size() - shaderSrcNameStart
                };

                size_t shaderSrcNameCharSize{ shaderSrcName.size() };
                Scope<char[]> shaderSrcNameChar{ new char[shaderSrcNameCharSize] };
                wcstombs_s(nullptr, shaderSrcNameChar.get(), shaderSrcNameCharSize, shaderSrcName.data(), shaderSrcName.size());

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

                const char* target{ "" };
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
                case ShaderType::Geometry:
                    target = "gs_5_0";
                    break;
                }

                if (FAILED(D3DCompile2(
                    shaderSrc.data(), shaderSrc.size(),
                    shaderSrcNameChar.get(),
                    defines.data(),
                    ShaderIncludeHandler::Get(),
                    spec.EntryPoint.c_str(),
                    target,
                    compileFlags, 0u,
                    0u, nullptr, 0u,
                    &shaderBlob,
                    &errorBlob
                )))
                {
                    throw std::runtime_error{ static_cast<const char*>(errorBlob->GetBufferPointer()) };
                }
            }
        }
    }

    void VertexShader::Create(const ShaderSpecification& spec)
    {
        m_VertexShaderBlob.Reset();
        m_VertexShader.Reset();

        m_Specification = spec;

        Utils::CompileShader(m_Specification, Utils::ShaderType::Vertex, m_VertexShaderBlob);

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateVertexShader(
            m_VertexShaderBlob->GetBufferPointer(),
            m_VertexShaderBlob->GetBufferSize(),
            nullptr,
            &m_VertexShader
        ));
    }

    void VertexShader::Bind() const noexcept
    {
        D3D::GetDeviceContext4()->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
    }

    void PixelShader::Create(const ShaderSpecification& spec)
    {
        m_PixelShaderBlob.Reset();
        m_PixelShader.Reset();

        m_Specification = spec;

        Utils::CompileShader(m_Specification, Utils::ShaderType::Pixel, m_PixelShaderBlob);

        DL_THROW_IF_HR(D3D::GetDevice5()->CreatePixelShader(
            m_PixelShaderBlob->GetBufferPointer(),
            m_PixelShaderBlob->GetBufferSize(),
            nullptr,
            &m_PixelShader
        ));
    }

    void PixelShader::Bind() const noexcept
    {
        D3D::GetDeviceContext4()->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
    }

    void HullShader::Create(const ShaderSpecification& spec)
    {
        m_HullShaderBlob.Reset();
        m_HullShader.Reset();

        m_Specification = spec;

        Utils::CompileShader(m_Specification, Utils::ShaderType::Hull, m_HullShaderBlob);

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateHullShader(
            m_HullShaderBlob->GetBufferPointer(),
            m_HullShaderBlob->GetBufferSize(),
            nullptr,
            &m_HullShader
        ));
    }

    void HullShader::Bind() const noexcept
    {
        D3D::GetDeviceContext4()->HSSetShader(m_HullShader.Get(), nullptr, 0u);
    }

    void DomainShader::Create(const ShaderSpecification& spec)
    {
        m_DomainShaderBlob.Reset();
        m_DomainShader.Reset();

        m_Specification = spec;

        Utils::CompileShader(m_Specification, Utils::ShaderType::Domain, m_DomainShaderBlob);

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateDomainShader(
            m_DomainShaderBlob->GetBufferPointer(),
            m_DomainShaderBlob->GetBufferSize(),
            nullptr,
            &m_DomainShader
        ));
    }

    void DomainShader::Bind() const noexcept
    {
        D3D::GetDeviceContext4()->DSSetShader(m_DomainShader.Get(), nullptr, 0u);
    }

    void GeometryShader::Create(const ShaderSpecification& spec)
    {
        m_GeometryShaderBlob.Reset();
        m_GeometryShader.Reset();

        m_Specification = spec;

        Utils::CompileShader(m_Specification, Utils::ShaderType::Geometry, m_GeometryShaderBlob);

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateGeometryShader(
            m_GeometryShaderBlob->GetBufferPointer(),
            m_GeometryShaderBlob->GetBufferSize(),
            nullptr,
            &m_GeometryShader
        ));
    }

    void GeometryShader::Bind() const noexcept
    {
        D3D::GetDeviceContext4()->GSSetShader(m_GeometryShader.Get(), nullptr, 0u);
    }

    ShaderIncludeHandler::~ShaderIncludeHandler()
    {
        delete this;
    }

    void ShaderIncludeHandler::Init() noexcept
    {
        DL_ASSERT(s_Instance == nullptr, "ShaderIncludeHandler already initialized");

        s_Instance = new ShaderIncludeHandler();

        s_Instance->AddIncludeDir(Filesystem::GetShaderDir());

        DL_LOG_INFO("ShaderIncludeHandler initialized");
    }

    STDOVERRIDEMETHODIMP ShaderIncludeHandler::Open(THIS_ D3D_INCLUDE_TYPE, LPCSTR pFileName, LPCVOID, LPCVOID* ppData, UINT* pBytes)
    {
        for (const auto& dir : m_IncludeDirs)
        {
            std::wstring path{ dir };
            path += std::wstring{ pFileName, pFileName + strlen(pFileName) };

            std::ifstream file{ path, std::ios::in | std::ios::binary | std::ios::ate };

            if (file.is_open())
            {
                const auto size{ file.tellg() };
                file.seekg(0, std::ios::beg);

                auto* data{ new char[size] };
                file.read(data, size);

                *ppData = data;
                *pBytes = static_cast<UINT>(size);

                return S_OK;
            }
        }

        return E_FAIL;
    }

    STDOVERRIDEMETHODIMP ShaderIncludeHandler::Close(THIS_ LPCVOID pData)
    {
        delete[] pData;
        return S_OK;
    }

}
