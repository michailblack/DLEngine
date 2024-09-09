#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    struct ShaderSpecification
    {
        std::vector<D3D_SHADER_MACRO> Defines{};
        std::wstring Path{};
        std::string EntryPoint{ "main" };
    };

    class VertexShader
    {
        friend class RenderCommand;
    public:
        void Create(const ShaderSpecification& spec);

        Microsoft::WRL::ComPtr<ID3DBlob> GetBlob() const { return m_Blob; }

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_Handle{};
        Microsoft::WRL::ComPtr<ID3DBlob> m_Blob{};
    };

    class PixelShader
    {
        friend class RenderCommand;
    public:
        void Create(const ShaderSpecification& spec);

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_Handle{};
        Microsoft::WRL::ComPtr<ID3DBlob> m_Blob{};
    };

    class HullShader
    {
        friend class RenderCommand;
    public:
        void Create(const ShaderSpecification& spec);

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3DBlob> m_Blob{};
        Microsoft::WRL::ComPtr<ID3D11HullShader> m_Handle{};
    };

    class DomainShader
    {
        friend class RenderCommand;
    public:
        void Create(const ShaderSpecification& spec);

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3D11DomainShader> m_Handle{};
        Microsoft::WRL::ComPtr<ID3DBlob> m_Blob{};
    };

    class GeometryShader
    {
        friend class RenderCommand;
    public:
        void Create(const ShaderSpecification& spec);

    private:
        ShaderSpecification m_Specification{};

        Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_Handle{};
        Microsoft::WRL::ComPtr<ID3DBlob> m_Blob{};
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
        void AddIncludeDir(const std::wstring& dir) noexcept { m_IncludeDirs.push_back(dir); }

    private:
        std::vector<std::wstring> m_IncludeDirs{};
    };
}
