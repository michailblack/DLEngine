#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    struct ShaderSpecification
    {
        std::vector<D3D_SHADER_MACRO> Defines{};
        std::string Path{};
        std::string EntryPoint{ "main" };
    };

    class VertexShader
    {
    public:
        void Create(const ShaderSpecification& spec);

        void Bind() const noexcept;

        Microsoft::WRL::ComPtr<ID3DBlob> GetBlob() const { return m_VertexShaderBlob; }

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3DBlob> m_VertexShaderBlob{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader{ nullptr };
    };

    class PixelShader
    {
    public:
        void Create(const ShaderSpecification& spec);

        void Bind() const noexcept;

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3DBlob> m_PixelShaderBlob{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader{ nullptr };
    };

    class HullShader
    {
    public:
        void Create(const ShaderSpecification& spec);

        void Bind() const noexcept;

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3DBlob> m_HullShaderBlob{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11HullShader> m_HullShader{ nullptr };
    };

    class DomainShader
    {
    public:
        void Create(const ShaderSpecification& spec);

        void Bind() const noexcept;

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3DBlob> m_DomainShaderBlob{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11DomainShader> m_DomainShader{ nullptr };
    };

    class GeometryShader
    {
    public:
        void Create(const ShaderSpecification& spec);

        void Bind() const noexcept;

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3DBlob> m_GeometryShaderBlob{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_GeometryShader{ nullptr };
    };

    class ShaderIncludeHandler
        : public ID3DInclude
    {
    public:
        ~ShaderIncludeHandler();
        
        STDOVERRIDEMETHODIMP Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override;

        STDOVERRIDEMETHODIMP Close(THIS_ LPCVOID pData) override;

        static void Init() noexcept;
        static ShaderIncludeHandler* Get() noexcept { return s_Instance; }

    private:
        ShaderIncludeHandler() = default;

        inline static ShaderIncludeHandler* s_Instance{ nullptr };

    private:
        void AddIncludeDir(const std::string& dir) noexcept { m_IncludeDirs.push_back(dir); }

    private:
        std::vector<std::string> m_IncludeDirs{};
    };
}
