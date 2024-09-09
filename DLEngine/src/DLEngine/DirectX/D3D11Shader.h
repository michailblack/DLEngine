#pragma once
#include "DLEngine/Renderer/Shader.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11Shader : public Shader
    {
    public:
        struct ReflectionData
        {
            std::unordered_map<std::string, ShaderBuffer> ConstantBuffers{};
            std::unordered_map<std::string, ShaderTexture> Textures{};
        };

    public:
        D3D11Shader(const ShaderSpecification& specification);
        ~D3D11Shader() override = default;

        const std::string& GetName() const noexcept override { return m_Name; }

        const VertexBufferLayout& GetInstanceLayout() const noexcept override { return m_Specification.InstanceLayout; }

        const ReflectionData& GetReflectionData() const noexcept { return m_ReflectionData; }

        Microsoft::WRL::ComPtr<ID3D11InputLayout> GetD3D11InputLayout() const noexcept { return m_D3D11InputLayout; }
        Microsoft::WRL::ComPtr<ID3D11VertexShader> GetD3D11VertexShader() const noexcept { return m_D3D11VertexShader; }
        Microsoft::WRL::ComPtr<ID3D11PixelShader> GetD3D11PixelShader() const noexcept { return m_D3D11PixelShader; }
        Microsoft::WRL::ComPtr<ID3D11HullShader> GetD3D11HullShader() const noexcept { return m_D3D11HullShader; }
        Microsoft::WRL::ComPtr<ID3D11DomainShader> GetD3D11DomainShader() const noexcept { return m_D3D11DomainShader; }
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> GetD3D11GeometryShader() const noexcept { return m_D3D11GeometryShader; }

    private:
        ShaderSpecification m_Specification;
        ReflectionData m_ReflectionData;
        std::string m_Name;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_D3D11InputLayout;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_D3D11VertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_D3D11PixelShader;
        Microsoft::WRL::ComPtr<ID3D11HullShader> m_D3D11HullShader;
        Microsoft::WRL::ComPtr<ID3D11DomainShader> m_D3D11DomainShader;
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_D3D11GeometryShader;

    private:
        friend class D3D11ShaderCompiler;
    };
}