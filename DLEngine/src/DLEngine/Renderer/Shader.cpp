#include "dlpch.h"
#include "Shader.h"

#include "DLEngine/Core/Application.h"

#include "DLEngine/DirectX/D3D11Shader.h"

#include "DLEngine/Renderer/Mesh/Mesh.h"

namespace DLEngine
{
    Ref<Shader> Shader::Create(const ShaderSpecification& specification)
    {
        return CreateRef<D3D11Shader>(specification);
    }

    const std::filesystem::path Shader::GetShaderDirectoryPath() noexcept
    {
        return Application::Get().GetWorkingDir() / "DLEngine\\src\\DLEngine\\Shaders\\";
    }

    void ShaderLibrary::Init()
    {
        ShaderSpecification textureOnlyShaderSpec{};
        textureOnlyShaderSpec.Path = Shader::GetShaderDirectoryPath() / "TextureOnly.hlsl";
        textureOnlyShaderSpec.VertexLayout = Mesh::GetCommonVertexBufferLayout();
        textureOnlyShaderSpec.InstanceLayout = VertexBufferLayout{
            { "TRANSFORM", ShaderDataType::Mat4 }
        };
        textureOnlyShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        textureOnlyShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(textureOnlyShaderSpec);

        ShaderSpecification pbrShaderSpec{};
        pbrShaderSpec.Path = Shader::GetShaderDirectoryPath() / "PBR_Static.hlsl";
        pbrShaderSpec.VertexLayout = Mesh::GetCommonVertexBufferLayout();
        pbrShaderSpec.InstanceLayout = VertexBufferLayout{
            { "TRANSFORM", ShaderDataType::Mat4 }
        };
        pbrShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        pbrShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(pbrShaderSpec);

        ShaderSpecification emissionShaderSpec{};
        emissionShaderSpec.Path = Shader::GetShaderDirectoryPath() / "Emission.hlsl";
        emissionShaderSpec.VertexLayout = Mesh::GetCommonVertexBufferLayout();
        emissionShaderSpec.InstanceLayout = VertexBufferLayout{
            { "TRANSFORM", ShaderDataType::Mat4 },
            { "RADIANCE" , ShaderDataType::Float3 },
        };
        emissionShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        emissionShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(emissionShaderSpec);

        ShaderSpecification skyboxShaderSpec{};
        skyboxShaderSpec.Path = Shader::GetShaderDirectoryPath() / "Skybox.hlsl";
        skyboxShaderSpec.VertexLayout = VertexBufferLayout{};
        skyboxShaderSpec.InstanceLayout = VertexBufferLayout{};
        skyboxShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        skyboxShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(skyboxShaderSpec);

        ShaderSpecification postProcessShaderSpec{};
        postProcessShaderSpec.Path = Shader::GetShaderDirectoryPath() / "PostProcess.hlsl";
        postProcessShaderSpec.VertexLayout = VertexBufferLayout{};
        postProcessShaderSpec.InstanceLayout = VertexBufferLayout{};
        postProcessShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        postProcessShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(postProcessShaderSpec);

        ShaderSpecification environmentIrradianceShaderSpec{};
        environmentIrradianceShaderSpec.Path = Shader::GetShaderDirectoryPath() / "EnvironmentIrradiance.hlsl";
        environmentIrradianceShaderSpec.VertexLayout = VertexBufferLayout{};
        environmentIrradianceShaderSpec.InstanceLayout = VertexBufferLayout{};
        environmentIrradianceShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        environmentIrradianceShaderSpec.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        environmentIrradianceShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(environmentIrradianceShaderSpec);

        ShaderSpecification prefilteredEnvironmentShaderSpec{};
        prefilteredEnvironmentShaderSpec.Path = Shader::GetShaderDirectoryPath() / "PrefilteredEnvironment.hlsl";
        prefilteredEnvironmentShaderSpec.VertexLayout = VertexBufferLayout{};
        prefilteredEnvironmentShaderSpec.InstanceLayout = VertexBufferLayout{};
        prefilteredEnvironmentShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        prefilteredEnvironmentShaderSpec.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        prefilteredEnvironmentShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(prefilteredEnvironmentShaderSpec);

        ShaderSpecification directionalLightShadowMapShaderSpec{};
        directionalLightShadowMapShaderSpec.Path = Shader::GetShaderDirectoryPath() / "DirectionalLightShadowMap.hlsl";
        directionalLightShadowMapShaderSpec.VertexLayout = Mesh::GetCommonVertexBufferLayout();
        directionalLightShadowMapShaderSpec.InstanceLayout = VertexBufferLayout{
            { "TRANSFORM", ShaderDataType::Mat4 }
        };
        directionalLightShadowMapShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        Load(directionalLightShadowMapShaderSpec);

        ShaderSpecification pointLightShadowMapShaderSpec{};
        pointLightShadowMapShaderSpec.Path = Shader::GetShaderDirectoryPath() / "OmnidirectionalLightShadowMap.hlsl";
        pointLightShadowMapShaderSpec.VertexLayout = Mesh::GetCommonVertexBufferLayout();
        pointLightShadowMapShaderSpec.InstanceLayout = VertexBufferLayout{
            { "TRANSFORM", ShaderDataType::Mat4 }
        };
        pointLightShadowMapShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        pointLightShadowMapShaderSpec.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        Load(pointLightShadowMapShaderSpec);

        ShaderSpecification dissolutionShaderSpec{};
        dissolutionShaderSpec.Path = Shader::GetShaderDirectoryPath() / "PBR_Static_Dissolution.hlsl";
        dissolutionShaderSpec.VertexLayout = Mesh::GetCommonVertexBufferLayout();
        dissolutionShaderSpec.InstanceLayout = VertexBufferLayout{
            { "TRANSFORM"           , ShaderDataType::Mat4  },
            { "DISSOLUTION_DURATION", ShaderDataType::Float },
            { "ELAPSED_TIME"        , ShaderDataType::Float }
        };
        dissolutionShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        dissolutionShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(dissolutionShaderSpec);
    }

    void ShaderLibrary::Add(const Ref<Shader>& shader) noexcept
    {
        DL_ASSERT(!m_Shaders.contains(shader->GetName()), "Shader [{0}] already added in the shader library", shader->GetName());

        m_Shaders[shader->GetName()] = shader;
    }

    Ref<Shader> ShaderLibrary::Load(const ShaderSpecification& specification) noexcept
    {
        Ref<Shader> shader{ Shader::Create(specification) };
        Add(shader);

        return shader;
    }

    Ref<Shader> ShaderLibrary::Get(const std::string_view name) noexcept
    {
        DL_ASSERT(m_Shaders.contains(name), "Shader [{0}] not found in the shader library", name);

        return m_Shaders[name];
    }

}