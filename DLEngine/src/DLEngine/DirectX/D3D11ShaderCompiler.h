#pragma once
#include "DLEngine/DirectX/D3D11Shader.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
    class D3D11ShaderIncludeHandler : public ID3DInclude
    {
    public:
        D3D11ShaderIncludeHandler(std::string baseDirectory);

        STDOVERRIDEMETHODIMP Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override;
        STDOVERRIDEMETHODIMP Close(THIS_ LPCVOID pData) override;

    private:
        std::string m_BaseDirectory;
        std::stack<std::string> m_DirectoryStack;
    };

    class D3D11ShaderCompiler
    {
    public:
        D3D11ShaderCompiler(D3D11Shader* const shader);

        void Compile();

    private:
        void PreProcess();
        void CompilePreProcessedSource(ShaderStage stage, std::string_view entryPoint, uint32_t compileFlags);
        void ReflectShaderStage(ShaderStage stage);
        void BuildInputLayout(const VertexBufferLayout& layout, uint32_t inputSlot, D3D11_INPUT_CLASSIFICATION inputSlotClass, uint32_t instanceDataStepRate);

    private:
        std::unordered_map<ShaderStage, Microsoft::WRL::ComPtr<ID3DBlob>> m_D3D11ShaderData;

        std::vector<D3D_SHADER_MACRO> m_D3D11ShaderMacros;
        std::vector<D3D11_INPUT_ELEMENT_DESC> m_D3D11InputLayoutDesc;
        
        Microsoft::WRL::ComPtr<ID3DBlob> m_D3D11ShaderSource;
        Microsoft::WRL::ComPtr<ID3DBlob> m_D3D11ShaderDebugData;

        Scope<D3D11ShaderIncludeHandler> m_D3D11ShaderIncludeHandler;

        D3D11Shader* m_D3D11Shader;
    };
}