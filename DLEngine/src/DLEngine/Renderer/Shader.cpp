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
        gBufferPBR_StaticSpecification.InputLayouts[2u] = {
            VertexBufferLayout{ { "INSTANCE_UUID", ShaderDataType::Uint2 }, }, InputLayoutType::PerInstance, 1u
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
                { "INSTANCE_UUID"       , ShaderDataType::Uint2 },
                { "DISSOLUTION_DURATION", ShaderDataType::Float },
                { "ELAPSED_TIME"        , ShaderDataType::Float }
            },
            InputLayoutType::PerInstance, 1u
        };
        gBufferPBR_Static_DissolutionSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        gBufferPBR_Static_DissolutionSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(gBufferPBR_Static_DissolutionSpecification);

        ShaderSpecification gBufferPBR_Static_IncinerationSpecification{};
        gBufferPBR_Static_IncinerationSpecification.Path = Shader::GetShaderDirectoryPath() / "GBuffer_PBR_Static_Incineration.hlsl";
        gBufferPBR_Static_IncinerationSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        gBufferPBR_Static_IncinerationSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        gBufferPBR_Static_IncinerationSpecification.InputLayouts[2u] = {
            VertexBufferLayout{
                { "INCINERATION_PARTICLE_EMISSION"          , ShaderDataType::Float3 },
                { "INCINERATION_SPHERE_POSITION_MESH_SPACE" , ShaderDataType::Float3 },
                { "INSTANCE_UUID"                           , ShaderDataType::Uint2  },
                { "MAX_INCINERATION_SPHERE_RADIUS"          , ShaderDataType::Float  },
                { "INCINERATION_DURATION"                   , ShaderDataType::Float  },
                { "ELAPSED_TIME"                            , ShaderDataType::Float  },
                { "PARTICLE_DISCARD_DIVISOR"                , ShaderDataType::Uint   }
            },
            InputLayoutType::PerInstance, 1u
        };
        gBufferPBR_Static_IncinerationSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        gBufferPBR_Static_IncinerationSpecification.EntryPoints[ShaderStage::DL_HULL_SHADER_BIT] = "mainHS";
        gBufferPBR_Static_IncinerationSpecification.EntryPoints[ShaderStage::DL_DOMAIN_SHADER_BIT] = "mainDS";
        gBufferPBR_Static_IncinerationSpecification.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        gBufferPBR_Static_IncinerationSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(gBufferPBR_Static_IncinerationSpecification);

        ShaderSpecification compute_IncinerationParticlesUpdateIndirectArgsSpecification{};
        compute_IncinerationParticlesUpdateIndirectArgsSpecification.Path = Shader::GetShaderDirectoryPath() / "Compute_IncinerationParticlesUpdateIndirectArgs.hlsl";
        compute_IncinerationParticlesUpdateIndirectArgsSpecification.EntryPoints[ShaderStage::DL_COMPUTE_SHADER_BIT] = "mainCS";
        Load(compute_IncinerationParticlesUpdateIndirectArgsSpecification);

        ShaderSpecification compute_IncinerationParticlesUpdateSpecification{};
        compute_IncinerationParticlesUpdateSpecification.Path = Shader::GetShaderDirectoryPath() / "Compute_IncinerationParticlesUpdate.hlsl";
        compute_IncinerationParticlesUpdateSpecification.EntryPoints[ShaderStage::DL_COMPUTE_SHADER_BIT] = "mainCS";
        Load(compute_IncinerationParticlesUpdateSpecification);

        ShaderSpecification compute_IncinerationParticlesAuxiliarySpecification{};
        compute_IncinerationParticlesAuxiliarySpecification.Path = Shader::GetShaderDirectoryPath() / "Compute_IncinerationParticlesAuxiliary.hlsl";
        compute_IncinerationParticlesAuxiliarySpecification.EntryPoints[ShaderStage::DL_COMPUTE_SHADER_BIT] = "mainCS";
        Load(compute_IncinerationParticlesAuxiliarySpecification);

        ShaderSpecification incinerationParticlesInfluenceSpecification{};
        incinerationParticlesInfluenceSpecification.Path = Shader::GetShaderDirectoryPath() / "IncinerationParticlesInfluence.hlsl";
        incinerationParticlesInfluenceSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        incinerationParticlesInfluenceSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(incinerationParticlesInfluenceSpecification);

        ShaderSpecification incinerationParticlesSpecification{};
        incinerationParticlesSpecification.Path = Shader::GetShaderDirectoryPath() / "IncinerationParticles.hlsl";
        incinerationParticlesSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        incinerationParticlesSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(incinerationParticlesSpecification);

        ShaderSpecification gBufferEmissionSpecification{};
        gBufferEmissionSpecification.Path = Shader::GetShaderDirectoryPath() / "GBuffer_Emission.hlsl";
        gBufferEmissionSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        gBufferEmissionSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        gBufferEmissionSpecification.InputLayouts[2u] = {
            VertexBufferLayout{
                { "RADIANCE"     , ShaderDataType::Float3 },
                { "INSTANCE_UUID", ShaderDataType::Uint2  }
            },
            InputLayoutType::PerInstance, 1u
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
        postProcessSpecification.Path = Shader::GetShaderDirectoryPath() / "HDR_To_LDR.hlsl";
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

        ShaderSpecification shadowMapDirectionalDissolutionSpecification{};
        shadowMapDirectionalDissolutionSpecification.Path = Shader::GetShaderDirectoryPath() / "ShadowMap_Directional_Dissolution.hlsl";
        shadowMapDirectionalDissolutionSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        shadowMapDirectionalDissolutionSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        shadowMapDirectionalDissolutionSpecification.InputLayouts[2u] = {
            VertexBufferLayout{
                { "INSTANCE_UUID"       , ShaderDataType::Uint2 },
                { "DISSOLUTION_DURATION", ShaderDataType::Float },
                { "ELAPSED_TIME"        , ShaderDataType::Float }
            },
            InputLayoutType::PerInstance, 1u
        };
        shadowMapDirectionalDissolutionSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        shadowMapDirectionalDissolutionSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(shadowMapDirectionalDissolutionSpecification);

        ShaderSpecification shadowMapDirectionalIncinerationSpecification{};
        shadowMapDirectionalIncinerationSpecification.Path = Shader::GetShaderDirectoryPath() / "ShadowMap_Directional_Incineration.hlsl";
        shadowMapDirectionalIncinerationSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        shadowMapDirectionalIncinerationSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        shadowMapDirectionalIncinerationSpecification.InputLayouts[2u] = {
            VertexBufferLayout{
                { "INCINERATION_PARTICLE_EMISSION"          , ShaderDataType::Float3 },
                { "INCINERATION_SPHERE_POSITION_MESH_SPACE" , ShaderDataType::Float3 },
                { "INSTANCE_UUID"                           , ShaderDataType::Uint2  },
                { "MAX_INCINERATION_SPHERE_RADIUS"          , ShaderDataType::Float  },
                { "INCINERATION_DURATION"                   , ShaderDataType::Float  },
                { "ELAPSED_TIME"                            , ShaderDataType::Float  },
            },
            InputLayoutType::PerInstance, 1u
        };
        shadowMapDirectionalIncinerationSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        shadowMapDirectionalIncinerationSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(shadowMapDirectionalIncinerationSpecification);

        ShaderSpecification shadowMapOmnidirectionalSpecification{};
        shadowMapOmnidirectionalSpecification.Path = Shader::GetShaderDirectoryPath() / "ShadowMap_Omnidirectional.hlsl";
        shadowMapOmnidirectionalSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        shadowMapOmnidirectionalSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        shadowMapOmnidirectionalSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        shadowMapOmnidirectionalSpecification.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        Load(shadowMapOmnidirectionalSpecification);

        ShaderSpecification shadowMapOmnidirectionalDissolutionSpecification{};
        shadowMapOmnidirectionalDissolutionSpecification.Path = Shader::GetShaderDirectoryPath() / "ShadowMap_Omnidirectional_Dissolution.hlsl";
        shadowMapOmnidirectionalDissolutionSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        shadowMapOmnidirectionalDissolutionSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        shadowMapOmnidirectionalDissolutionSpecification.InputLayouts[2u] = {
            VertexBufferLayout{
                { "INSTANCE_UUID"       , ShaderDataType::Uint2 },
                { "DISSOLUTION_DURATION", ShaderDataType::Float },
                { "ELAPSED_TIME"        , ShaderDataType::Float }
            },
            InputLayoutType::PerInstance, 1u
        };
        shadowMapOmnidirectionalDissolutionSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        shadowMapOmnidirectionalDissolutionSpecification.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        shadowMapOmnidirectionalDissolutionSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(shadowMapOmnidirectionalDissolutionSpecification);

        ShaderSpecification shadowMapOmnidirectionalIncinerationSpecification{};
        shadowMapOmnidirectionalIncinerationSpecification.Path = Shader::GetShaderDirectoryPath() / "ShadowMap_Omnidirectional_Incineration.hlsl";
        shadowMapOmnidirectionalIncinerationSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        shadowMapOmnidirectionalIncinerationSpecification.InputLayouts[1u] = {
            VertexBufferLayout{ { "TRANSFORM", ShaderDataType::Mat4 } }, InputLayoutType::PerInstance, 1u
        };
        shadowMapOmnidirectionalIncinerationSpecification.InputLayouts[2u] = {
            VertexBufferLayout{
                { "INCINERATION_PARTICLE_EMISSION"          , ShaderDataType::Float3 },
                { "INCINERATION_SPHERE_POSITION_MESH_SPACE" , ShaderDataType::Float3 },
                { "INSTANCE_UUID"                           , ShaderDataType::Uint2  },
                { "MAX_INCINERATION_SPHERE_RADIUS"          , ShaderDataType::Float  },
                { "INCINERATION_DURATION"                   , ShaderDataType::Float  },
                { "ELAPSED_TIME"                            , ShaderDataType::Float  },
            },
            InputLayoutType::PerInstance, 1u
        };
        shadowMapOmnidirectionalIncinerationSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        shadowMapOmnidirectionalIncinerationSpecification.EntryPoints[ShaderStage::DL_GEOMETRY_SHADER_BIT] = "mainGS";
        shadowMapOmnidirectionalIncinerationSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(shadowMapOmnidirectionalIncinerationSpecification);

        ShaderSpecification smokeParticleSpecification{};
        smokeParticleSpecification.Path = Shader::GetShaderDirectoryPath() / "SmokeParticle.hlsl";
        smokeParticleSpecification.InputLayouts[0u] = {
            VertexBufferLayout{
                { "PARTICLE_WORLD_POS"         , ShaderDataType::Float3 },
                { "PARTICLE_TINT_COLOR"        , ShaderDataType::Float3 },
                { "PARTICLE_INITIAL_SIZE"      , ShaderDataType::Float2 },
                { "PARTICLE_END_SIZE"          , ShaderDataType::Float2 },
                { "PARTICLE_EMISSION_INTENSITY", ShaderDataType::Float  },
                { "PARTICLE_LIFETIME_MS"       , ShaderDataType::Float  },
                { "PARTICLE_LIFETIME_PASSED_MS", ShaderDataType::Float  },
                { "PARTICLE_ROTATION"          , ShaderDataType::Float  }
            },
            InputLayoutType::PerInstance, 1u
        };
        smokeParticleSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        smokeParticleSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(smokeParticleSpecification);

        ShaderSpecification decalSpecification{};
        decalSpecification.Path = Shader::GetShaderDirectoryPath() / "GBuffer_Decal.hlsl";
        decalSpecification.InputLayouts[0u] = { Mesh::GetCommonVertexBufferLayout(), InputLayoutType::PerVertex, 0u };
        decalSpecification.InputLayouts[1u] = {
            VertexBufferLayout{
                { "DECAL_TO_WORLD", ShaderDataType::Mat4 },
                { "WORLD_TO_DECAL", ShaderDataType::Mat4 }
            },
            InputLayoutType::PerInstance, 1u,
        };
        decalSpecification.InputLayouts[2u] = {
            VertexBufferLayout{
                { "DECAL_TINT_COLOR"    , ShaderDataType::Float3 },
                { "PARENT_INSTANCE_UUID", ShaderDataType::Uint2  },
            },
            InputLayoutType::PerInstance, 1u
        };
        decalSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        decalSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(decalSpecification);

        ShaderSpecification fxaaSpecification{};
        fxaaSpecification.Path = Shader::GetShaderDirectoryPath() / "FXAA.hlsl";
        fxaaSpecification.EntryPoints[ShaderStage::DL_VERTEX_SHADER_BIT] = "mainVS";
        fxaaSpecification.EntryPoints[ShaderStage::DL_PIXEL_SHADER_BIT] = "mainPS";
        Load(fxaaSpecification);
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