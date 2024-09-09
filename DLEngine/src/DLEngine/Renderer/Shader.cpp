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
        ShaderSpecification gBufferPBR_StaticSpecification{};
        gBufferPBR_StaticSpecification.Path = Shader::GetShaderDirectoryPath() / "GBuffer_PBR_Static.hlsl";
        gBufferPBR_StaticSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        gBufferPBR_StaticSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        gBufferPBR_StaticSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        gBufferPBR_StaticSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(gBufferPBR_StaticSpecification);

        ShaderSpecification gBufferPBR_Static_DissolutionSpecification{};
        gBufferPBR_Static_DissolutionSpecification.Path = Shader::GetShaderDirectoryPath() / "GBuffer_PBR_Static_Dissolution.hlsl";
        gBufferPBR_Static_DissolutionSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        gBufferPBR_Static_DissolutionSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        gBufferPBR_Static_DissolutionSpecification.InputLayouts[2u] = {
            VertexBufferLayout{
                { "DISSOLUTION_DURATION", ShaderDataType::Float },
                { "ELAPSED_TIME"        , ShaderDataType::Float }
            },
            InputLayoutType::PerInstance, 1u
        };
        gBufferPBR_Static_DissolutionSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        gBufferPBR_Static_DissolutionSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(gBufferPBR_Static_DissolutionSpecification);

        ShaderSpecification gBufferEmissionSpecification{};
        gBufferEmissionSpecification.Path = Shader::GetShaderDirectoryPath() / "GBuffer_Emission.hlsl";
        gBufferEmissionSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        gBufferEmissionSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        gBufferEmissionSpecification.InputLayouts[2u] = {
            VertexBufferLayout{ { "RADIANCE", ShaderDataType::Float3 } }, InputLayoutType::PerInstance, 1u
        };
        gBufferEmissionSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        gBufferEmissionSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(gBufferEmissionSpecification);

        ShaderSpecification gBufferResolvePBR_StaticSpecification{};
        gBufferResolvePBR_StaticSpecification.Path = Shader::GetShaderDirectoryPath() / "GBufferResolve_PBR_Static.hlsl";
        gBufferResolvePBR_StaticSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        gBufferResolvePBR_StaticSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(gBufferResolvePBR_StaticSpecification);

        ShaderSpecification gBufferResolveEmissionSpecification{};
        gBufferResolveEmissionSpecification.Path = Shader::GetShaderDirectoryPath() / "GBufferResolve_Emission.hlsl";
        gBufferResolveEmissionSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        gBufferResolveEmissionSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(gBufferResolveEmissionSpecification);

        ShaderSpecification skyboxSpecification{};
        skyboxSpecification.Path = Shader::GetShaderDirectoryPath() / "Skybox.hlsl";
        skyboxSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        skyboxSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(skyboxSpecification);

        ShaderSpecification postProcessSpecification{};
        postProcessSpecification.Path = Shader::GetShaderDirectoryPath() / "PostProcess.hlsl";
        postProcessSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        postProcessSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(postProcessSpecification);

        ShaderSpecification environmentIrradianceSpecification{};
        environmentIrradianceSpecification.Path = Shader::GetShaderDirectoryPath() / "EnvironmentIrradiance.hlsl";
        environmentIrradianceSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        environmentIrradianceSpecification.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        environmentIrradianceSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(environmentIrradianceSpecification);

        ShaderSpecification prefilteredEnvironmentSpecification{};
        prefilteredEnvironmentSpecification.Path = Shader::GetShaderDirectoryPath() / "PrefilteredEnvironment.hlsl";
        prefilteredEnvironmentSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        prefilteredEnvironmentSpecification.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        prefilteredEnvironmentSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(prefilteredEnvironmentSpecification);

        ShaderSpecification shadowMapDirectionalSpecification{};
        shadowMapDirectionalSpecification.Path = Shader::GetShaderDirectoryPath() / "ShadowMap_Directional.hlsl";
        shadowMapDirectionalSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        shadowMapDirectionalSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        shadowMapDirectionalSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        Load(shadowMapDirectionalSpecification);

        ShaderSpecification shadowMapOmnidirectionalSpecification{};
        shadowMapOmnidirectionalSpecification.Path = Shader::GetShaderDirectoryPath() / "ShadowMap_Omnidirectional.hlsl";
        shadowMapOmnidirectionalSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        shadowMapOmnidirectionalSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        shadowMapOmnidirectionalSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        shadowMapOmnidirectionalSpecification.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        Load(shadowMapOmnidirectionalSpecification);

        /*ShaderSpecification smokeParticleShaderSpec{};
        smokeParticleShaderSpec.Path = Shader::GetShaderDirectoryPath() / "SmokeParticle.hlsl";
        smokeParticleShaderSpec.VertexLayout = VertexBufferLayout{};
        smokeParticleShaderSpec.InstanceLayout = VertexBufferLayout{
            { "PARTICLE_WORLD_POS"         , ShaderDataType::Float3 },
            { "PARTICLE_TINT_COLOR"        , ShaderDataType::Float3 },
            { "PARTICLE_INITIAL_SIZE"      , ShaderDataType::Float2 },
            { "PARTICLE_END_SIZE"          , ShaderDataType::Float2 },
            { "PARTICLE_EMISSION_INTENSITY", ShaderDataType::Float },
            { "PARTICLE_LIFETIME_MS"       , ShaderDataType::Float },
            { "PARTICLE_LIFETIME_PASSED_MS", ShaderDataType::Float },
            { "PARTICLE_ROTATION"          , ShaderDataType::Float }
        };
        smokeParticleShaderSpec.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        smokeParticleShaderSpec.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(smokeParticleShaderSpec);*/
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