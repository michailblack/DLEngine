#include "WorldLayer.h"

#include "DLEngine/Core/Filesystem.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/D3DStates.h"
#include "DLEngine/DirectX/View.h"

#include "DLEngine/Renderer/PostProcess.h"
#include "DLEngine/Renderer/Renderer.h"
#include "DLEngine/Renderer/TextureManager.h"

#include "DLEngine/Systems/Light/LightSystem.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"
#include "DLEngine/Systems/Mesh/ModelManager.h"

#include "DLEngine/Systems/Transform/TransformSystem.h"

struct HologramGroupInstance
{
    DLEngine::Math::Vec3 BaseColor{};
    DLEngine::Math::Vec3 AdditionalColor{};
};

struct TextureOnlyGroupMaterial
{
    DLEngine::ShaderResourceView TextureSRV{};

    void Set() const noexcept
    {
        TextureSRV.Bind(0u, DLEngine::BIND_PS);
    }

    bool operator==(const TextureOnlyGroupMaterial& other) const
    {
        return TextureSRV.Handle == other.TextureSRV.Handle;
    }
};

struct EmissionGroupInstance
{
    DLEngine::Math::Vec3 EmissionColor{};
};

struct LitGroupMaterial
{
    DLEngine::ShaderResourceView AlbedoSRV{};
    DLEngine::ShaderResourceView NormalSRV{};
    DLEngine::ShaderResourceView MetallicSRV{};
    DLEngine::ShaderResourceView RoughnessSRV{};

    void Set() const noexcept
    {
        AlbedoSRV.Bind(0u, DLEngine::BIND_PS);
        NormalSRV.Bind(1u, DLEngine::BIND_PS);
        MetallicSRV.Bind(2u, DLEngine::BIND_PS);
        RoughnessSRV.Bind(3u, DLEngine::BIND_PS);
    }

    bool operator==(const LitGroupMaterial& other) const
    {
        return AlbedoSRV.Handle == other.AlbedoSRV.Handle &&
            NormalSRV.Handle == other.NormalSRV.Handle &&
            MetallicSRV.Handle == other.MetallicSRV.Handle &&
            RoughnessSRV.Handle == other.RoughnessSRV.Handle;
    }
};

WorldLayer::WorldLayer()
    : m_CameraController(DLEngine::Camera { DLEngine::Math::ToRadians(45.0f), 800.0f / 600.0f, 0.001f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    const auto cube{ DLEngine::ModelManager::Load(DLEngine::Filesystem::GetModelDir() + L"cube\\cube.obj") };
    const auto samurai{ DLEngine::ModelManager::Load(DLEngine::Filesystem::GetModelDir() + L"samurai\\samurai.fbx") };
    const auto flashlight{ DLEngine::ModelManager::Load(DLEngine::Filesystem::GetModelDir() + L"flashlight\\flashlight.fbx") };
    const auto sphere { DLEngine::ModelManager::Get(L"UNIT_SPHERE") };
    
    const auto skybox{ DLEngine::TextureManager::LoadTexture2D(DLEngine::Filesystem::GetTextureDir() + L"skybox\\night_street_hdr.dds") };
    DLEngine::Renderer::SetSkybox(skybox.SRV);

    uint32_t transformID{ 0u };

    InitHologramGroup();
    InitTextureOnlyGroup();
    InitEmissionGroup();
    InitLitGroup();
    
    std::vector<DLEngine::NullMaterial> nullMaterials{};
    std::vector<TextureOnlyGroupMaterial> textureOnlyMaterials{};
    std::vector<LitGroupMaterial> litMaterials{};

    nullMaterials.resize(sphere->GetMeshesCount());

    DLEngine::PointLight pointLight{};
    pointLight.Position = DLEngine::Math::Vec3{ 0.0f };
    pointLight.Radius = 0.5f;
    pointLight.Luminance = DLEngine::Math::Vec3{ 98.3f, 156.7f, 30.4f };

    transformID = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Scale(DLEngine::Math::Vec3{ pointLight.Radius }) *
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 1.5f, 2.0f, 1.0f })
    );

    DLEngine::LightSystem::AddPointLight(pointLight, transformID);

    EmissionGroupInstance emissionInstance{};
    emissionInstance.EmissionColor = pointLight.Luminance;

    DLEngine::MeshSystem::Get().Add<>(sphere, nullMaterials, emissionInstance, transformID);

    pointLight.Position = DLEngine::Math::Vec3{ 0.0f };
    pointLight.Radius = 0.1f;
    pointLight.Luminance = DLEngine::Math::Vec3{ 23.1f, 41.5f, 99.9f };

    transformID = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Scale(DLEngine::Math::Vec3{ pointLight.Radius }) *
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -1.5f, 2.0f, 1.0f })
    );

    DLEngine::LightSystem::AddPointLight(pointLight, transformID);

    emissionInstance.EmissionColor = pointLight.Luminance;

    DLEngine::MeshSystem::Get().Add<>(sphere, nullMaterials, emissionInstance, transformID);

    litMaterials.resize(samurai->GetMeshesCount());
    transformID = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -1.5f, 0.0f, 1.5f })
    );

    litMaterials[0].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Sword_BaseColor.dds"
    ).SRV;
    litMaterials[0].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Sword_Normal.dds"
    ).SRV;
    litMaterials[0].MetallicSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Sword_Metallic.dds"
    ).SRV;
    litMaterials[0].RoughnessSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Sword_Roughness.dds"
    ).SRV;

    litMaterials[1].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Head_BaseColor.dds"
    ).SRV;
    litMaterials[1].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Head_Normal.dds"
    ).SRV;
    litMaterials[1].MetallicSRV = DLEngine::TextureManager::GenerateValueTexture2D(DLEngine::Math::Vec4{ 0.0f }).SRV;
    litMaterials[1].RoughnessSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Head_Roughness.dds"
    ).SRV;

    litMaterials[2].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Eyes_BaseColor.dds"
    ).SRV;
    litMaterials[2].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Eyes_Normal.dds"
    ).SRV;
    litMaterials[2].MetallicSRV = DLEngine::TextureManager::GenerateValueTexture2D(DLEngine::Math::Vec4{ 0.0f }).SRV;
    litMaterials[2].RoughnessSRV = DLEngine::TextureManager::GenerateValueTexture2D(DLEngine::Math::Vec4{ 0.0f }).SRV;

    litMaterials[3].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Helmet_BaseColor.dds"
    ).SRV;
    litMaterials[3].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Helmet_Normal.dds"
    ).SRV;
    litMaterials[3].MetallicSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Helmet_Metallic.dds"
    ).SRV;
    litMaterials[3].RoughnessSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Helmet_Roughness.dds"
    ).SRV;

    litMaterials[4].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Decor_BaseColor.dds"
    ).SRV;
    litMaterials[4].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Decor_Normal.dds"
    ).SRV;
    litMaterials[4].MetallicSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Decor_Metallic.dds"
    ).SRV;
    litMaterials[4].RoughnessSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Decor_Roughness.dds"
    ).SRV;

    litMaterials[5].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Pants_BaseColor.dds"
    ).SRV;
    litMaterials[5].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Pants_Normal.dds"
    ).SRV;
    litMaterials[5].MetallicSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Pants_Metallic.dds"
    ).SRV;
    litMaterials[5].RoughnessSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Pants_Roughness.dds"
    ).SRV;

    litMaterials[6].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Hands_BaseColor.dds"
    ).SRV;
    litMaterials[6].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Hands_Normal.dds"
    ).SRV;
    litMaterials[6].MetallicSRV = DLEngine::TextureManager::GenerateValueTexture2D(DLEngine::Math::Vec4{ 0.0f }).SRV;
    litMaterials[6].RoughnessSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Hands_Roughness.dds"
    ).SRV;

    litMaterials[7].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Torso_BaseColor.dds"
    ).SRV;
    litMaterials[7].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Torso_Normal.dds"
    ).SRV;
    litMaterials[7].MetallicSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Torso_Metallic.dds"
    ).SRV;
    litMaterials[7].RoughnessSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"samurai\\Torso_Roughness.dds"
    ).SRV;

    DLEngine::MeshSystem::Get().Add<>(samurai, litMaterials, DLEngine::NullInstance{}, transformID);

    transformID = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 1.5f, 0.0f, 1.5f })
    );
    DLEngine::MeshSystem::Get().Add<>(samurai, litMaterials, DLEngine::NullInstance{}, transformID);

    litMaterials.resize(cube->GetMeshesCount());
    transformID = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -1.5f, 0.0f, 0.0f })
    );

    litMaterials[0].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"cube\\Cobblestone_albedo.dds"
    ).SRV;
    litMaterials[0].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"cube\\Cobblestone_normal.dds"
    ).SRV;
    litMaterials[0].MetallicSRV = DLEngine::TextureManager::GenerateValueTexture2D(DLEngine::Math::Vec4{ 0.0f }).SRV;
    litMaterials[0].RoughnessSRV = DLEngine::TextureManager::GenerateValueTexture2D(DLEngine::Math::Vec4{ 0.0f }).SRV;

    for (int32_t x{ -5 }; x < 5; ++x)
    {
        for (int32_t z{ -5 }; z < 5; ++z)
        {
            transformID = DLEngine::TransformSystem::AddTransform(
                DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ static_cast<float>(x), -0.5f, static_cast<float>(z) })
            );
            DLEngine::MeshSystem::Get().Add<>(cube, litMaterials, DLEngine::NullInstance{}, transformID);
        }
    }

    DLEngine::DirectionalLight directionalLight{};
    directionalLight.Direction = DLEngine::Math::Normalize(DLEngine::Math::Vec3{ -1.0f, -1.0f, 1.0f });
    directionalLight.SolidAngle = 6.418e-5f;
    directionalLight.Luminance = DLEngine::Math::Vec3{ 0.05f, 0.06f, 0.1f };

    transformID = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Identity()
    );

    DLEngine::LightSystem::AddDirectionalLight(directionalLight, transformID);

    m_CameraTransformID = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Identity()
    );

    DLEngine::SpotLight spotLight{};
    spotLight.Position = DLEngine::Math::Vec3{ 0.0f, 0.0f, 0.0f };
    spotLight.Direction = DLEngine::Math::Normalize(DLEngine::Math::Vec3{ 0.0f, 0.0f, 1.0f });
    spotLight.Luminance = DLEngine::Math::Vec3{ 400.0f, 100.0f, 1500.0f };
    spotLight.Radius = 0.075f;
    spotLight.InnerCutoffCos = DLEngine::Math::Cos(DLEngine::Math::ToRadians(15.0f));
    spotLight.OuterCutoffCos = DLEngine::Math::Cos(DLEngine::Math::ToRadians(25.0f));

    DLEngine::LightSystem::AddSpotLight(spotLight, m_CameraTransformID);

    litMaterials.resize(flashlight->GetMeshesCount());
    litMaterials[0].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Base_color.dds"
    ).SRV;
    litMaterials[0].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Normal.dds"
    ).SRV;
    litMaterials[0].MetallicSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Metallic.dds"
    ).SRV;
    litMaterials[0].RoughnessSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Roughness.dds"
    ).SRV;

    litMaterials[1].AlbedoSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Base_color.dds"
    ).SRV;
    litMaterials[1].NormalSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Normal.dds"
    ).SRV;
    litMaterials[1].MetallicSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Metallic.dds"
    ).SRV;
    litMaterials[1].RoughnessSRV = DLEngine::TextureManager::LoadTexture2D(
        DLEngine::Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Roughness.dds"
    ).SRV;

    DLEngine::MeshSystem::Get().Add<>(flashlight, litMaterials, DLEngine::NullInstance{}, m_CameraTransformID);
}

void WorldLayer::OnDetach()
{
    
}

void WorldLayer::OnUpdate(DeltaTime dt)
{
    m_CameraController.OnUpdate(dt);

    if (m_IsFlashlightAttached)
    {
        DLEngine::TransformSystem::ReplaceTransform(
            m_CameraTransformID,
            DLEngine::Math::Mat4x4::Scale(DLEngine::Math::Vec3{ 0.003f }) *
            DLEngine::Math::Mat4x4::Inverse(m_CameraController.GetCamera().GetViewMatrix()) *
            DLEngine::Math::Mat4x4::Translate(m_CameraController.GetCamera().GetForward() * 0.5f) *
            DLEngine::Math::Mat4x4::Translate(m_CameraController.GetCamera().GetRight() * 0.2f) *
            DLEngine::Math::Mat4x4::Translate(m_CameraController.GetCamera().GetUp() * -0.2f)
        );
    }

    static constexpr float EV100Step{ 0.001f };
    if (DLEngine::Input::IsKeyPressed(VK_OEM_PLUS))
    {
        m_EV100 += EV100Step * dt;
        if (m_EV100 > 16.0f)
            m_EV100 = 16.0f;

        DL_LOG_INFO("EV100: {0}", m_EV100);
    }
    else if (DLEngine::Input::IsKeyPressed(VK_OEM_MINUS))
    {
        m_EV100 -= EV100Step * dt;
        if (m_EV100 < -16.0f)
            m_EV100 = -16.0f;

        DL_LOG_INFO("EV100: {0}", m_EV100);
    }

    DLEngine::PostProcessSettings postProcessSettings{};
    postProcessSettings.EV100 = m_EV100;
    
    DLEngine::Renderer::SetPostProcessSettings(postProcessSettings);
    DLEngine::Renderer::BeginScene(m_CameraController.GetCamera());

    DLEngine::Renderer::EndScene();
}

void WorldLayer::OnEvent(DLEngine::Event& e)
{
    m_CameraController.OnEvent(e);

    DLEngine::EventDispatcher dispatcher{ e };
    dispatcher.Dispatch<DLEngine::KeyPressedEvent>(DL_BIND_EVENT_FN(WorldLayer::OnKeyPressedEvent));
}

void WorldLayer::InitHologramGroup() const
{
    using namespace DLEngine;
    ShadingGroupDesc hologramGroupDesc{};

    hologramGroupDesc.Name = "Hologram";

    hologramGroupDesc.PipelineDesc.Topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

    hologramGroupDesc.InstanceBufferLayout = BufferLayout{
        { "BASE_COLOR", BufferLayout::ShaderDataType::Float3 },
        { "ADD_COLOR" , BufferLayout::ShaderDataType::Float3 }
    };

    ShaderSpecification shaderSpec{};
    shaderSpec.Path = Filesystem::GetShaderDir() + L"Hologram.hlsl";

    shaderSpec.EntryPoint = "mainVS";
    VertexShader vs{};
    vs.Create(shaderSpec);
    hologramGroupDesc.PipelineDesc.VS = vs;

    shaderSpec.EntryPoint = "mainPS";
    PixelShader ps{};
    ps.Create(shaderSpec);
    hologramGroupDesc.PipelineDesc.PS = ps;

    shaderSpec.EntryPoint = "mainHS";
    HullShader hs{};
    hs.Create(shaderSpec);
    hologramGroupDesc.PipelineDesc.HS = hs;

    shaderSpec.EntryPoint = "mainDS";
    DomainShader ds{};
    ds.Create(shaderSpec);
    hologramGroupDesc.PipelineDesc.DS = ds;
    
    shaderSpec.EntryPoint = "mainGS";
    GeometryShader gs{};
    gs.Create(shaderSpec);
    hologramGroupDesc.PipelineDesc.GS = gs;

    hologramGroupDesc.PipelineDesc.DepthStencil = D3DStates::GetDepthStencilState(DLEngine::DepthStencilStates::DEFAULT);
    hologramGroupDesc.PipelineDesc.Rasterizer = D3DStates::GetRasterizerState(DLEngine::RasterizerStates::DEFAULT);

    MeshSystem::Get().CreateShadingGroup<NullMaterial, HologramGroupInstance>(hologramGroupDesc);
}

void WorldLayer::InitTextureOnlyGroup() const
{
    using namespace DLEngine;
    ShadingGroupDesc textureOnlyGroupDesc{};

    textureOnlyGroupDesc.Name = "TextureOnly";

    textureOnlyGroupDesc.PipelineDesc.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    textureOnlyGroupDesc.InstanceBufferLayout = BufferLayout{
        { "_empty", BufferLayout::ShaderDataType::Float }
    };

    ShaderSpecification shaderSpec{};
    shaderSpec.Path = Filesystem::GetShaderDir() + L"TextureOnly.hlsl";

    shaderSpec.EntryPoint = "mainVS";
    VertexShader vs{};
    vs.Create(shaderSpec);
    textureOnlyGroupDesc.PipelineDesc.VS = vs;

    shaderSpec.EntryPoint = "mainPS";
    PixelShader ps{};
    ps.Create(shaderSpec);
    textureOnlyGroupDesc.PipelineDesc.PS = ps;

    textureOnlyGroupDesc.PipelineDesc.DepthStencil = D3DStates::GetDepthStencilState(DLEngine::DepthStencilStates::DEFAULT);
    textureOnlyGroupDesc.PipelineDesc.Rasterizer = D3DStates::GetRasterizerState(DLEngine::RasterizerStates::DEFAULT);

    MeshSystem::Get().CreateShadingGroup<TextureOnlyGroupMaterial, NullInstance>(textureOnlyGroupDesc);
}

void WorldLayer::InitEmissionGroup() const
{
    using namespace DLEngine;

    ShadingGroupDesc emissionGroupDesc{};

    emissionGroupDesc.Name = "Emission";

    emissionGroupDesc.PipelineDesc.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    emissionGroupDesc.InstanceBufferLayout = BufferLayout{
        { "EMISSION_LUMINANCE", BufferLayout::ShaderDataType::Float3 }
    };

    ShaderSpecification shaderSpec{};
    shaderSpec.Path = Filesystem::GetShaderDir() + L"Emission.hlsl";

    shaderSpec.EntryPoint = "mainVS";
    VertexShader vs{};
    vs.Create(shaderSpec);
    emissionGroupDesc.PipelineDesc.VS = vs;

    shaderSpec.EntryPoint = "mainPS";
    PixelShader ps{};
    ps.Create(shaderSpec);
    emissionGroupDesc.PipelineDesc.PS = ps;

    emissionGroupDesc.PipelineDesc.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEFAULT);
    emissionGroupDesc.PipelineDesc.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);

    MeshSystem::Get().CreateShadingGroup<NullMaterial, EmissionGroupInstance>(emissionGroupDesc);
}

void WorldLayer::InitLitGroup() const
{
    using namespace DLEngine;

    ShadingGroupDesc litGroupDesc{};

    litGroupDesc.Name = "Lit";

    litGroupDesc.PipelineDesc.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    litGroupDesc.InstanceBufferLayout = BufferLayout{
        { "_empty", BufferLayout::ShaderDataType::Float }
    };

    ShaderSpecification shaderSpec{};
    shaderSpec.Path = Filesystem::GetShaderDir() + L"Lit.hlsl";

    shaderSpec.EntryPoint = "mainVS";
    VertexShader vs{};
    vs.Create(shaderSpec);
    litGroupDesc.PipelineDesc.VS = vs;

    shaderSpec.EntryPoint = "mainPS";
    PixelShader ps{};
    ps.Create(shaderSpec);
    litGroupDesc.PipelineDesc.PS = ps;

    litGroupDesc.PipelineDesc.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEFAULT);
    litGroupDesc.PipelineDesc.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::DEFAULT);

    MeshSystem::Get().CreateShadingGroup<LitGroupMaterial, NullInstance>(litGroupDesc);
}

bool WorldLayer::OnKeyPressedEvent(DLEngine::KeyPressedEvent& e)
{
    if (e.GetKeyCode() == 'F')
        m_IsFlashlightAttached = !m_IsFlashlightAttached;

    return false;
}
