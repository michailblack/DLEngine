#include "WorldLayer.h"

#include "DLEngine/Core/Filesystem.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/Renderer/PostProcess.h"
#include "DLEngine/Renderer/Renderer.h"
#include "DLEngine/Renderer/TextureManager.h"

#include "DLEngine/Systems/Light/LightSystem.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"
#include "DLEngine/Systems/Mesh/ModelManager.h"

#include "DLEngine/Systems/Transform/TransformSystem.h"

WorldLayer::WorldLayer()
    : m_CameraController(DLEngine::Camera { DLEngine::Math::ToRadians(45.0f), 800.0f / 600.0f, 0.001f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    using namespace DLEngine;

    const auto cube{ ModelManager::Load(Filesystem::GetModelDir() + L"cube\\cube.obj") };
    const auto samurai{ ModelManager::Load(Filesystem::GetModelDir() + L"samurai\\samurai.fbx") };
    const auto flashlight{ ModelManager::Load(Filesystem::GetModelDir() + L"flashlight\\flashlight.fbx") };
    const auto sphere { ModelManager::Get(L"UNIT_SPHERE") };
    
    const auto skybox{ TextureManager::LoadTexture2D(Filesystem::GetTextureDir() + L"skybox\\night_street_hdr.dds") };
    Renderer::SetSkybox(skybox.GetSRV());

    uint32_t transformID{ 0u };

    std::vector<ShadingGroupStruct::Material::Null> nullMaterials{};
    std::vector<ShadingGroupStruct::Material::TextureOnly> textureOnlyMaterials{};
    std::vector<ShadingGroupStruct::Material::Lit> litMaterials{};

    nullMaterials.resize(sphere->GetMeshesCount());

    PointLight pointLight{};
    pointLight.Position = Math::Vec3{ 0.0f };
    pointLight.Radius = 0.5f;
    pointLight.Radiance = Math::Vec3{ 98.3f, 156.7f, 30.4f };

    transformID = TransformSystem::AddTransform(
        Math::Mat4x4::Scale(Math::Vec3{ pointLight.Radius }) *
        Math::Mat4x4::Translate(Math::Vec3{ 1.5f, 2.0f, 1.0f })
    );

    ShadingGroupStruct::Instance::Emission emissionInstance{};
    emissionInstance.PointLightID = LightSystem::AddPointLight(pointLight, transformID);
    emissionInstance.TransformID = transformID;

    MeshSystem::Get().Add<>(sphere, nullMaterials, emissionInstance);

    pointLight.Position = Math::Vec3{ 0.0f };
    pointLight.Radius = 0.1f;
    pointLight.Radiance = Math::Vec3{ 23.1f, 41.5f, 99.9f };

    transformID = TransformSystem::AddTransform(
        Math::Mat4x4::Scale(Math::Vec3{ pointLight.Radius }) *
        Math::Mat4x4::Translate(Math::Vec3{ -1.5f, 2.0f, 1.0f })
    );

    emissionInstance.PointLightID = LightSystem::AddPointLight(pointLight, transformID);
    emissionInstance.TransformID = transformID;

    MeshSystem::Get().Add<>(sphere, nullMaterials, emissionInstance);

    litMaterials.resize(samurai->GetMeshesCount());
    transformID = TransformSystem::AddTransform(
        Math::Mat4x4::Translate(Math::Vec3{ -1.5f, 0.0f, 1.5f })
    );

    litMaterials[0].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Sword_BaseColor.dds"
    ).GetSRV();
    litMaterials[0].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Sword_Normal.dds"
    ).GetSRV();
    litMaterials[0].MetallicSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Sword_Metallic.dds"
    ).GetSRV();
    litMaterials[0].RoughnessSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Sword_Roughness.dds"
    ).GetSRV();

    litMaterials[1].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Head_BaseColor.dds"
    ).GetSRV();
    litMaterials[1].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Head_Normal.dds"
    ).GetSRV();
    litMaterials[1].MetallicSRV = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f }).GetSRV();
    litMaterials[1].RoughnessSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Head_Roughness.dds"
    ).GetSRV();

    litMaterials[2].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Eyes_BaseColor.dds"
    ).GetSRV();
    litMaterials[2].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Eyes_Normal.dds"
    ).GetSRV();
    litMaterials[2].MetallicSRV = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f }).GetSRV();
    litMaterials[2].RoughnessSRV = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f }).GetSRV();

    litMaterials[3].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Helmet_BaseColor.dds"
    ).GetSRV();
    litMaterials[3].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Helmet_Normal.dds"
    ).GetSRV();
    litMaterials[3].MetallicSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Helmet_Metallic.dds"
    ).GetSRV();
    litMaterials[3].RoughnessSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Helmet_Roughness.dds"
    ).GetSRV();

    litMaterials[4].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Decor_BaseColor.dds"
    ).GetSRV();
    litMaterials[4].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Decor_Normal.dds"
    ).GetSRV();
    litMaterials[4].MetallicSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Decor_Metallic.dds"
    ).GetSRV();
    litMaterials[4].RoughnessSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Decor_Roughness.dds"
    ).GetSRV();

    litMaterials[5].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Pants_BaseColor.dds"
    ).GetSRV();
    litMaterials[5].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Pants_Normal.dds"
    ).GetSRV();
    litMaterials[5].MetallicSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Pants_Metallic.dds"
    ).GetSRV();
    litMaterials[5].RoughnessSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Pants_Roughness.dds"
    ).GetSRV();

    litMaterials[6].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Hands_BaseColor.dds"
    ).GetSRV();
    litMaterials[6].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Hands_Normal.dds"
    ).GetSRV();
    litMaterials[6].MetallicSRV = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f }).GetSRV();
    litMaterials[6].RoughnessSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Hands_Roughness.dds"
    ).GetSRV();

    litMaterials[7].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Torso_BaseColor.dds"
    ).GetSRV();
    litMaterials[7].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Torso_Normal.dds"
    ).GetSRV();
    litMaterials[7].MetallicSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Torso_Metallic.dds"
    ).GetSRV();
    litMaterials[7].RoughnessSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"samurai\\Torso_Roughness.dds"
    ).GetSRV();

    ShadingGroupStruct::Instance::Null nullInstance{};
    nullInstance.TransformID = transformID;

    MeshSystem::Get().Add<>(samurai, litMaterials, nullInstance);

    transformID = TransformSystem::AddTransform(
        Math::Mat4x4::Translate(Math::Vec3{ 1.5f, 0.0f, 1.5f })
    );
    nullInstance.TransformID = transformID;
    MeshSystem::Get().Add<>(samurai, litMaterials, nullInstance);

    litMaterials.resize(cube->GetMeshesCount());
    transformID = TransformSystem::AddTransform(
        Math::Mat4x4::Translate(Math::Vec3{ -1.5f, 0.0f, 0.0f })
    );

    litMaterials[0].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"cube\\Cobblestone_albedo.dds"
    ).GetSRV();
    litMaterials[0].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"cube\\Cobblestone_normal.dds"
    ).GetSRV();
    litMaterials[0].MetallicSRV = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f }).GetSRV();
    litMaterials[0].RoughnessSRV = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f }).GetSRV();

    for (int32_t x{ -5 }; x < 5; ++x)
    {
        for (int32_t z{ -5 }; z < 5; ++z)
        {
            transformID = TransformSystem::AddTransform(
                Math::Mat4x4::Translate(Math::Vec3{ static_cast<float>(x), -0.5f, static_cast<float>(z) })
            );
            nullInstance.TransformID = transformID;
            MeshSystem::Get().Add<>(cube, litMaterials, nullInstance);
        }
    }

    DirectionalLight directionalLight{};
    directionalLight.Direction = Math::Normalize(Math::Vec3{ -1.0f, -1.0f, 1.0f });
    directionalLight.SolidAngle = 6.418e-5f;
    directionalLight.Radiance = Math::Vec3{ 0.05f, 0.06f, 0.1f };

    transformID = TransformSystem::AddTransform(
        Math::Mat4x4::Identity()
    );

    LightSystem::AddDirectionalLight(directionalLight, transformID);

    m_CameraTransformID = TransformSystem::AddTransform(
        Math::Mat4x4::Identity()
    );

    SpotLight spotLight{};
    spotLight.Position = Math::Vec3{ 0.0f, 0.0f, 0.0f };
    spotLight.Direction = Math::Normalize(Math::Vec3{ 0.0f, 0.0f, 1.0f });
    spotLight.Radiance = Math::Vec3{ 400.0f, 100.0f, 1500.0f };
    spotLight.Radius = 0.075f;
    spotLight.InnerCutoffCos = Math::Cos(Math::ToRadians(15.0f));
    spotLight.OuterCutoffCos = Math::Cos(Math::ToRadians(25.0f));

    LightSystem::AddSpotLight(spotLight, m_CameraTransformID);

    litMaterials.resize(flashlight->GetMeshesCount());
    litMaterials[0].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Base_color.dds"
    ).GetSRV();
    litMaterials[0].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Normal.dds"
    ).GetSRV();
    litMaterials[0].MetallicSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Metallic.dds"
    ).GetSRV();
    litMaterials[0].RoughnessSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Roughness.dds"
    ).GetSRV();

    litMaterials[1].AlbedoSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Base_color.dds"
    ).GetSRV();
    litMaterials[1].NormalSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Normal.dds"
    ).GetSRV();
    litMaterials[1].MetallicSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Metallic.dds"
    ).GetSRV();
    litMaterials[1].RoughnessSRV = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"flashlight\\Flashlight_Roughness.dds"
    ).GetSRV();

    nullInstance.TransformID = m_CameraTransformID;
    MeshSystem::Get().Add<>(flashlight, litMaterials, nullInstance);
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

bool WorldLayer::OnKeyPressedEvent(DLEngine::KeyPressedEvent& e)
{
    if (e.GetKeyCode() == 'F')
        m_IsFlashlightAttached = !m_IsFlashlightAttached;

    return false;
}
