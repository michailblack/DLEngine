#include "dlpch.h"
#include "D3D11ShaderCompiler.h"

#include "DLEngine/DirectX/D3D11Context.h"
#include "DLEngine/DirectX/D3D11Shader.h"

#include "DLEngine/Renderer/Renderer.h"

#include <d3dcompiler.h>
#include <d3d11shader.h>

#pragma comment(lib, "dxguid.lib")

namespace DLEngine
{
    namespace Utils
    {
        ShaderDataType ShaderDataTypeFromD3D11TypeDesc(const D3D11_SHADER_TYPE_DESC& typeDesc);
        ShaderTextureType ShaderTextureTypeFromD3D11Dimension(const D3D_SRV_DIMENSION& dimension);
        DXGI_FORMAT DXGIFormatFromShaderDataType(ShaderDataType type);
    }

    D3D11ShaderIncludeHandler::D3D11ShaderIncludeHandler(std::string baseDirectory)
        : m_BaseDirectory(baseDirectory)
    {
        m_DirectoryStack.push(m_BaseDirectory);
    }

    STDOVERRIDEMETHODIMP D3D11ShaderIncludeHandler::Open(THIS_ D3D_INCLUDE_TYPE, LPCSTR pFileName, LPCVOID, LPCVOID* ppData, UINT* pBytes)
    {
        std::filesystem::path filePath{ m_DirectoryStack.top() };
        filePath /= pFileName;

        std::ifstream file{ filePath, std::ios::binary | std::ios::ate };
        if (!file.is_open())
            return E_FAIL;

        std::streamsize fileSize{ file.tellg() };
        file.seekg(0, std::ios::beg);

        char* fileData{ new char[static_cast<size_t>(fileSize)] };
        file.read(fileData, fileSize);

        *ppData = fileData;
        *pBytes = static_cast<UINT>(fileSize);

        m_DirectoryStack.push(filePath.parent_path().string());

        return S_OK;
    }

    STDOVERRIDEMETHODIMP D3D11ShaderIncludeHandler::Close(THIS_ LPCVOID pData)
    {
        m_DirectoryStack.pop();

        delete[] reinterpret_cast<const char*>(pData);

        return S_OK;
    }

    D3D11ShaderCompiler::D3D11ShaderCompiler(D3D11Shader* const shader)
        : m_D3D11ShaderIncludeHandler(CreateScope<D3D11ShaderIncludeHandler>(shader->m_Specification.Path.parent_path().string()))
        , m_D3D11Shader(shader)
    {
    }

    void D3D11ShaderCompiler::Compile()
    {
        const auto& device{ D3D11Context::Get()->GetDevice5() };

        const auto& specification{ m_D3D11Shader->m_Specification };

        m_D3D11ShaderMacros.reserve(specification.Defines.size() + 1u);

        for (const auto& macro : specification.Defines)
            m_D3D11ShaderMacros.push_back({ macro.Name.c_str(), macro.Value.c_str() });

        m_D3D11ShaderMacros.push_back({ nullptr, nullptr });

        PreProcess();

        UINT compileFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR |
            D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef DL_DEBUG
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION /*| D3DCOMPILE_WARNINGS_ARE_ERRORS*/;
#else
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        if (specification.EntryPoints.contains(ShaderStage::DL_VERTEX_SHADER_BIT))
        {
            const auto entryPoint{ specification.EntryPoints.at(ShaderStage::DL_VERTEX_SHADER_BIT) };
            CompilePreProcessedSource(ShaderStage::DL_VERTEX_SHADER_BIT, entryPoint, compileFlags);
            DL_THROW_IF_HR(device->CreateVertexShader(
                m_D3D11ShaderData[ShaderStage::DL_VERTEX_SHADER_BIT]->GetBufferPointer(),
                m_D3D11ShaderData[ShaderStage::DL_VERTEX_SHADER_BIT]->GetBufferSize(),
                nullptr,
                &m_D3D11Shader->m_D3D11VertexShader
            ));
        }

        if (specification.EntryPoints.contains(ShaderStage::DL_PIXEL_SHADER_BIT))
        {
            const auto entryPoint{ specification.EntryPoints.at(ShaderStage::DL_PIXEL_SHADER_BIT) };
            CompilePreProcessedSource(ShaderStage::DL_PIXEL_SHADER_BIT, entryPoint, compileFlags);
            DL_THROW_IF_HR(device->CreatePixelShader(
                m_D3D11ShaderData[ShaderStage::DL_PIXEL_SHADER_BIT]->GetBufferPointer(),
                m_D3D11ShaderData[ShaderStage::DL_PIXEL_SHADER_BIT]->GetBufferSize(),
                nullptr,
                &m_D3D11Shader->m_D3D11PixelShader
            ));
        }

        if (specification.EntryPoints.contains(ShaderStage::DL_HULL_SHADER_BIT))
        {
            const auto entryPoint{ specification.EntryPoints.at(ShaderStage::DL_HULL_SHADER_BIT) };
            CompilePreProcessedSource(ShaderStage::DL_HULL_SHADER_BIT, entryPoint, compileFlags);
            DL_THROW_IF_HR(device->CreateHullShader(
                m_D3D11ShaderData[ShaderStage::DL_HULL_SHADER_BIT]->GetBufferPointer(),
                m_D3D11ShaderData[ShaderStage::DL_HULL_SHADER_BIT]->GetBufferSize(),
                nullptr,
                &m_D3D11Shader->m_D3D11HullShader
            ));
        }

        if (specification.EntryPoints.contains(ShaderStage::DL_DOMAIN_SHADER_BIT))
        {
            const auto entryPoint{ specification.EntryPoints.at(ShaderStage::DL_DOMAIN_SHADER_BIT) };
            CompilePreProcessedSource(ShaderStage::DL_DOMAIN_SHADER_BIT, entryPoint, compileFlags);
            DL_THROW_IF_HR(device->CreateDomainShader(
                m_D3D11ShaderData[ShaderStage::DL_DOMAIN_SHADER_BIT]->GetBufferPointer(),
                m_D3D11ShaderData[ShaderStage::DL_DOMAIN_SHADER_BIT]->GetBufferSize(),
                nullptr,
                &m_D3D11Shader->m_D3D11DomainShader
            ));
        }

        if (specification.EntryPoints.contains(ShaderStage::DL_GEOMETRY_SHADER_BIT))
        {
            const auto entryPoint{ specification.EntryPoints.at(ShaderStage::DL_GEOMETRY_SHADER_BIT) };
            CompilePreProcessedSource(ShaderStage::DL_GEOMETRY_SHADER_BIT, entryPoint, compileFlags);
            DL_THROW_IF_HR(device->CreateGeometryShader(
                m_D3D11ShaderData[ShaderStage::DL_GEOMETRY_SHADER_BIT]->GetBufferPointer(),
                m_D3D11ShaderData[ShaderStage::DL_GEOMETRY_SHADER_BIT]->GetBufferSize(),
                nullptr,
                &m_D3D11Shader->m_D3D11GeometryShader
            ));
        }

        if (specification.EntryPoints.contains(ShaderStage::DL_COMPUTE_SHADER_BIT))
        {
            bool hasRasterizerPipelineShader{ false };
            hasRasterizerPipelineShader |= specification.EntryPoints.contains(ShaderStage::DL_VERTEX_SHADER_BIT);
            hasRasterizerPipelineShader |= specification.EntryPoints.contains(ShaderStage::DL_PIXEL_SHADER_BIT);
            hasRasterizerPipelineShader |= specification.EntryPoints.contains(ShaderStage::DL_HULL_SHADER_BIT);
            hasRasterizerPipelineShader |= specification.EntryPoints.contains(ShaderStage::DL_DOMAIN_SHADER_BIT);
            hasRasterizerPipelineShader |= specification.EntryPoints.contains(ShaderStage::DL_GEOMETRY_SHADER_BIT);

            DL_ASSERT(!hasRasterizerPipelineShader,
                "Compute shader does not belong to Rasetrizer Pipeline and must be compiled separately.\nFile: {0}",
                m_D3D11Shader->m_Specification.Path.string()
            );

            const auto entryPoint{ specification.EntryPoints.at(ShaderStage::DL_COMPUTE_SHADER_BIT) };
            CompilePreProcessedSource(ShaderStage::DL_COMPUTE_SHADER_BIT, entryPoint, compileFlags);
            DL_THROW_IF_HR(device->CreateComputeShader(
                m_D3D11ShaderData[ShaderStage::DL_COMPUTE_SHADER_BIT]->GetBufferPointer(),
                m_D3D11ShaderData[ShaderStage::DL_COMPUTE_SHADER_BIT]->GetBufferSize(),
                nullptr,
                &m_D3D11Shader->m_D3D11ComputeShader
            ));
        }

        for (const auto& [stage, data] : m_D3D11ShaderData)
            ReflectShaderStage(stage);

        for (const auto& [bindingPoint, inputLayoutEntry] : m_D3D11Shader->m_Specification.InputLayouts)
            BuildInputLayout(inputLayoutEntry.Layout, bindingPoint, inputLayoutEntry.Type, inputLayoutEntry.InstanceStepRate);

        if (m_D3D11InputLayoutDesc.empty())
            return;

        DL_THROW_IF_HR(device->CreateInputLayout(
            m_D3D11InputLayoutDesc.data(), static_cast<uint32_t>(m_D3D11InputLayoutDesc.size()),
            m_D3D11ShaderData[ShaderStage::DL_VERTEX_SHADER_BIT]->GetBufferPointer(),
            m_D3D11ShaderData[ShaderStage::DL_VERTEX_SHADER_BIT]->GetBufferSize(),
            &m_D3D11Shader->m_D3D11InputLayout
        ));
    }

    void D3D11ShaderCompiler::PreProcess()
    {
        DL_THROW_IF_HR(D3DReadFileToBlob(m_D3D11Shader->m_Specification.Path.wstring().c_str(), &m_D3D11ShaderSource));       
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob{};

        HRESULT hr{ D3DPreprocess(
            m_D3D11ShaderSource->GetBufferPointer(),
            m_D3D11ShaderSource->GetBufferSize(),
            m_D3D11Shader->m_Specification.Path.string().c_str(),
            m_D3D11ShaderMacros.data(),
            m_D3D11ShaderIncludeHandler.get(),
            &m_D3D11ShaderDebugData,
            &errorBlob
        ) };

        if (FAILED(hr))
        {
            DL_ASSERT(false,
                "Shader preprocessing failed for {0}.\nError(s):\n{1}",
                m_D3D11Shader->m_Specification.Path.string(),
                static_cast<const char*>(errorBlob->GetBufferPointer())
            );
        }
    }

    void D3D11ShaderCompiler::CompilePreProcessedSource(ShaderStage stage, std::string_view entryPoint, uint32_t compileFlags)
    {
        std::string target{};
        switch (stage)
        {
        case ShaderStage::DL_VERTEX_SHADER_BIT:
            target = "vs_5_0";
            break;
        case ShaderStage::DL_PIXEL_SHADER_BIT:
            target = "ps_5_0";
            break;
        case ShaderStage::DL_HULL_SHADER_BIT:
            target = "hs_5_0";
            break;
        case ShaderStage::DL_DOMAIN_SHADER_BIT:
            target = "ds_5_0";
            break;
        case ShaderStage::DL_GEOMETRY_SHADER_BIT:
            target = "gs_5_0";
            break;
        case ShaderStage::DL_COMPUTE_SHADER_BIT:
            target = "cs_5_0";
            break;
        default:
            DL_ASSERT(false);
            break;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob{};

        HRESULT hr{ D3DCompile2(
            m_D3D11ShaderDebugData->GetBufferPointer(),
            m_D3D11ShaderDebugData->GetBufferSize(),
            nullptr,
            nullptr,
            nullptr,
            entryPoint.data(),
            target.c_str(),
            compileFlags,
            0u,
            0u,
            nullptr,
            0u,
            &m_D3D11ShaderData[stage],
            &errorBlob
        ) };

        if (FAILED(hr))
        {
            DL_ASSERT(false, "Shader compilation failed for {0}.\nError(s):\n{1}",
                m_D3D11Shader->m_Specification.Path.string(),
                static_cast<const char*>(errorBlob->GetBufferPointer())
            );
        }
    }

    void D3D11ShaderCompiler::ReflectShaderStage(ShaderStage stage)
    {
        Microsoft::WRL::ComPtr<ID3D11ShaderReflection> shaderReflection{};

        DL_THROW_IF_HR(D3DReflect(
            m_D3D11ShaderData[stage]->GetBufferPointer(),
            m_D3D11ShaderData[stage]->GetBufferSize(),
            IID_ID3D11ShaderReflection,
            &shaderReflection
        ));

        D3D11_SHADER_DESC shaderDesc{};
        DL_THROW_IF_HR(shaderReflection->GetDesc(&shaderDesc));

        for (uint32_t i{ 0u }; i < shaderDesc.BoundResources; ++i)
        {
            D3D11_SHADER_INPUT_BIND_DESC bindDesc{};
            DL_THROW_IF_HR(shaderReflection->GetResourceBindingDesc(i, &bindDesc));

            switch (bindDesc.Type)
            {
            case D3D_SIT_CBUFFER:
            {
                ID3D11ShaderReflectionConstantBuffer* constantBuffer{ shaderReflection->GetConstantBufferByName(bindDesc.Name) };
                
                D3D11_SHADER_BUFFER_DESC bufferDesc{};
                DL_THROW_IF_HR(constantBuffer->GetDesc(&bufferDesc));

                auto& constantBuffers{ m_D3D11Shader->m_ReflectionData.ConstantBuffers };
                const std::string bufferName{ bufferDesc.Name };

                if (constantBuffers.contains(bufferName))
                {
                    constantBuffers[bufferName].ShaderStageFlags |= stage;
                    continue;
                }

                ShaderBuffer shaderBuffer{};
                shaderBuffer.Name = bufferName;
                shaderBuffer.Size = bufferDesc.Size;
                shaderBuffer.BindPoint = bindDesc.BindPoint;
                shaderBuffer.ShaderStageFlags = stage;
                
                shaderBuffer.Uniforms.reserve(bufferDesc.Variables);

                for (uint32_t j{ 0u }; j < bufferDesc.Variables; ++j)
                {
                    ID3D11ShaderReflectionVariable* variable{ constantBuffer->GetVariableByIndex(j) };
                    ID3D11ShaderReflectionType* type{ variable->GetType() };

                    D3D11_SHADER_VARIABLE_DESC varDesc{};
                    DL_THROW_IF_HR(variable->GetDesc(&varDesc));

                    D3D11_SHADER_TYPE_DESC typeDesc{};
                    DL_THROW_IF_HR(type->GetDesc(&typeDesc));

                    const std::string varName{ varDesc.Name };

                    shaderBuffer.Uniforms.emplace(std::make_pair(
                        varName,
                        ShaderUniform{ varName, Utils::ShaderDataTypeFromD3D11TypeDesc(typeDesc), varDesc.Size, varDesc.StartOffset }
                    ));
                }

                constantBuffers.emplace(std::make_pair(bufferName, shaderBuffer));
            } break;
            case D3D_SIT_TEXTURE:
            {
                if (bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
                    break;

                auto& textures{ m_D3D11Shader->m_ReflectionData.Textures };

                if (textures.contains(bindDesc.Name))
                {
                    textures[bindDesc.Name].ShaderStageFlags |= stage;
                    continue;
                }

                const std::string textureName{ bindDesc.Name };

                ShaderTexture texture{};
                texture.Name = textureName;
                texture.Type = Utils::ShaderTextureTypeFromD3D11Dimension(bindDesc.Dimension);
                texture.BindPoint = bindDesc.BindPoint;
                texture.ShaderStageFlags = stage;

                textures.emplace(std::make_pair(textureName, texture));
            } break;
            default:
                break;
            }
        }
    }

    void D3D11ShaderCompiler::BuildInputLayout(const VertexBufferLayout& layout, uint32_t slot, InputLayoutType type, uint32_t instanceDataStepRate)
    {
        m_D3D11InputLayoutDesc.reserve(m_D3D11InputLayoutDesc.size() + layout.GetElements().size());

        D3D11_INPUT_ELEMENT_DESC elementDesc{};
        elementDesc.InputSlot = slot;
        elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        elementDesc.InputSlotClass = type == InputLayoutType::PerVertex ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
        elementDesc.InstanceDataStepRate = instanceDataStepRate;

        for (const auto& bufferElement : layout)
        {
            elementDesc.SemanticName = bufferElement.Name.c_str();
            elementDesc.Format = Utils::DXGIFormatFromShaderDataType(bufferElement.Type);

            switch (bufferElement.Type)
            {
            case ShaderDataType::Mat3:
            {
                for (uint32_t i{ 0u }; i < 3u; ++i)
                {
                    elementDesc.SemanticIndex = i;
                    m_D3D11InputLayoutDesc.push_back(elementDesc);
                }
            } break;
            case ShaderDataType::Mat4:
            {
                for (uint32_t i{ 0u }; i < 4u; ++i)
                {
                    elementDesc.SemanticIndex = i;
                    m_D3D11InputLayoutDesc.push_back(elementDesc);
                }
            } break;
            default:
                elementDesc.SemanticIndex = 0u;
                m_D3D11InputLayoutDesc.push_back(elementDesc);
                break;
            }
        }
    }

    namespace Utils
    {
        ShaderDataType ShaderDataTypeFromD3D11TypeDesc(const D3D11_SHADER_TYPE_DESC& typeDesc)
        {
            switch (typeDesc.Type)
            {
            case D3D_SVT_FLOAT:
            {
                switch (typeDesc.Rows)
                {
                case 1:
                    switch (typeDesc.Columns)
                    {
                    case 1: return ShaderDataType::Float;
                    case 2: return ShaderDataType::Float2;
                    case 3: return ShaderDataType::Float3;
                    case 4: return ShaderDataType::Float4;
                    }
                case 3:
                    switch (typeDesc.Columns)
                    {
                    case 3: return ShaderDataType::Mat3;
                    }
                case 4:
                    switch (typeDesc.Columns)
                    {
                    case 4: return ShaderDataType::Mat4;
                    }
                }
            }
            case D3D_SVT_INT:
            {
                switch (typeDesc.Rows)
                {
                case 1:
                    switch (typeDesc.Columns)
                    {
                    case 1: return ShaderDataType::Int;
                    case 2: return ShaderDataType::Int2;
                    case 3: return ShaderDataType::Int3;
                    case 4: return ShaderDataType::Int4;
                    }
                }
            }
            case D3D_SVT_UINT:
            {
                switch (typeDesc.Rows)
                {
                case 1:
                    switch (typeDesc.Columns)
                    {
                    case 1: return ShaderDataType::Uint;
                    case 2: return ShaderDataType::Uint2;
                    case 3: return ShaderDataType::Uint3;
                    case 4: return ShaderDataType::Uint4;
                    }
                }
            }
            case D3D_SVT_BOOL:
            {
                switch (typeDesc.Rows)
                {
                case 1:
                    switch (typeDesc.Columns)
                    {
                    case 1: return ShaderDataType::Bool;
                    }
                }
            default:
                DL_ASSERT(false);
                return ShaderDataType::None;
            }
            }
        }

        ShaderTextureType ShaderTextureTypeFromD3D11Dimension(const D3D_SRV_DIMENSION& dimension)
        {
            switch (dimension)
            {
            case D3D_SRV_DIMENSION_TEXTURE2D:
            case D3D_SRV_DIMENSION_TEXTURE2DMS:      return ShaderTextureType::Texture2D;

            case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
            case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY: return ShaderTextureType::Texture2DArray;

            case D3D_SRV_DIMENSION_TEXTURECUBE:      return ShaderTextureType::TextureCube;

            case D3D_SRV_DIMENSION_TEXTURECUBEARRAY: return ShaderTextureType::TextureCubeArray;
            default: DL_ASSERT(false); return ShaderTextureType::None;
            }
        }

        DXGI_FORMAT DXGIFormatFromShaderDataType(ShaderDataType type)
        {
            switch (type)
            {
            case ShaderDataType::Float:  return DXGI_FORMAT_R32_FLOAT;
            case ShaderDataType::Float2: return DXGI_FORMAT_R32G32_FLOAT;

            case ShaderDataType::Mat3:
            case ShaderDataType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;

            case ShaderDataType::Mat4:
            case ShaderDataType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case ShaderDataType::Int:    return DXGI_FORMAT_R32_SINT;
            case ShaderDataType::Int2:   return DXGI_FORMAT_R32G32_SINT;
            case ShaderDataType::Int3:   return DXGI_FORMAT_R32G32B32_SINT;
            case ShaderDataType::Int4:   return DXGI_FORMAT_R32G32B32A32_SINT;

            case ShaderDataType::Bool:
            case ShaderDataType::Uint:   return DXGI_FORMAT_R32_UINT;

            case ShaderDataType::Uint2:  return DXGI_FORMAT_R32G32_UINT;
            case ShaderDataType::Uint3:  return DXGI_FORMAT_R32G32B32_UINT;
            case ShaderDataType::Uint4:  return DXGI_FORMAT_R32G32B32A32_UINT;

            case ShaderDataType::None:
            default:
                DL_ASSERT(false);
                return DXGI_FORMAT_UNKNOWN;
            }
        }
    }
}