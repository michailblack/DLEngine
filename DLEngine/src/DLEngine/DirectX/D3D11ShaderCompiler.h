#pragma once
#include "DLEngine/DirectX/D3D11Shader.h"

#include <d3d11_4.h>
#include <wrl.h>

namespace DLEngine
{
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
        D3D11Shader* m_D3D11Shader;
        
        std::vector<D3D_SHADER_MACRO> m_D3D11ShaderMacros;
        std::vector<D3D11_INPUT_ELEMENT_DESC> m_D3D11InputLayoutDesc;
        
        std::unordered_map<ShaderStage, Microsoft::WRL::ComPtr<ID3DBlob>> m_D3D11ShaderData;
        Microsoft::WRL::ComPtr<ID3DBlob> m_D3D11ShaderSource;
        Microsoft::WRL::ComPtr<ID3DBlob> m_D3D11ShaderDebugData;
    };
}