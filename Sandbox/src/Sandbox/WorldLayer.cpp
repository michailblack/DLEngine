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
    const auto skybox{ TextureManager::LoadTexture2D(Filesystem::GetTextureDir() + L"skybox\\night_street.dds") };

    Renderer::SetSkybox(skybox);

    uint32_t transformID{ 0u };

    std::vector<ShadingGroupStruct::Material::Null> nullMaterials{};
    std::vector<ShadingGroupStruct::Material::TextureOnly> textureOnlyMaterials{};
    std::vector<ShadingGroupStruct::Material::Lit> litMaterials{};

    nullMaterials.resize(sphere->GetMeshesCount());

    PointLight pointLight{};
    pointLight.Position = Math::Vec3{ 0.0f };
    pointLight.Radius = 0.5f;
    pointLight.Radiance = Math::Vec3{ 9.83f, 15.67f, 3.04f };

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

    litMaterials[0].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Sword_BaseColor.dds"
    );
    litMaterials[0].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Sword_Normal.dds"
    );
    litMaterials[0].Metallic = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Sword_Metallic.dds"
    );
    litMaterials[0].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Sword_Roughness.dds"
    );

    litMaterials[1].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Head_BaseColor.dds"
    );
    litMaterials[1].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Head_Normal.dds"
    );
    litMaterials[1].Metallic = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f });
    litMaterials[1].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Head_Roughness.dds"
    );

    litMaterials[2].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Eyes_BaseColor.dds"
    );
    litMaterials[2].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Eyes_Normal.dds"
    );
    litMaterials[2].Metallic = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f });
    litMaterials[2].Roughness = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f });

    litMaterials[3].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Helmet_BaseColor.dds"
    );
    litMaterials[3].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Helmet_Normal.dds"
    );
    litMaterials[3].Metallic = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Helmet_Metallic.dds"
    );
    litMaterials[3].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Helmet_Roughness.dds"
    );

    litMaterials[4].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Decor_BaseColor.dds"
    );
    litMaterials[4].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Decor_Normal.dds"
    );
    litMaterials[4].Metallic = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Decor_Metallic.dds"
    );
    litMaterials[4].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Decor_Roughness.dds"
    );

    litMaterials[5].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Pants_BaseColor.dds"
    );
    litMaterials[5].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Pants_Normal.dds"
    );
    litMaterials[5].Metallic = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Pants_Metallic.dds"
    );
    litMaterials[5].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Pants_Roughness.dds"
    );

    litMaterials[6].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Hands_BaseColor.dds"
    );
    litMaterials[6].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Hands_Normal.dds"
    );
    litMaterials[6].Metallic = TextureManager::GenerateValueTexture2D(Math::Vec4{ 0.0f });
    litMaterials[6].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Hands_Roughness.dds"
    );

    litMaterials[7].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Torso_BaseColor.dds"
    );
    litMaterials[7].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Torso_Normal.dds"
    );
    litMaterials[7].Metallic = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Torso_Metallic.dds"
    );
    litMaterials[7].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\samurai\\Torso_Roughness.dds"
    );

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

    litMaterials[0].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\cube\\metal_steel\\MetalSteelBrushed_BaseColor.dds"
    );
    litMaterials[0].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\cube\\metal_steel\\MetalSteelBrushed_Normal.dds"
    );
    litMaterials[0].Metallic = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\cube\\metal_steel\\MetalSteelBrushed_Metallic.dds"
    );
    litMaterials[0].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\cube\\metal_steel\\MetalSteelBrushed_Roughness.dds"
    );

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
    spotLight.Position = Math::Vec3{ 0.0f, 5.0f, 0.0f };
    spotLight.Direction = Math::Normalize(Math::Vec3{ 0.0f, 0.0f, 1.0f });
    spotLight.Radiance = Math::Vec3{ 40.0f, 10.0f, 150.0f };
    spotLight.Radius = 0.075f;
    spotLight.InnerCutoffCos = Math::Cos(Math::ToRadians(15.0f));
    spotLight.OuterCutoffCos = Math::Cos(Math::ToRadians(25.0f));

    LightSystem::AddSpotLight(spotLight, m_CameraTransformID);

    litMaterials.resize(flashlight->GetMeshesCount());
    litMaterials[0].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\flashlight\\Flashlight_Base_color.dds"
    );
    litMaterials[0].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\flashlight\\Flashlight_Normal.dds"
    );
    litMaterials[0].Metallic = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\flashlight\\Flashlight_Metallic.dds"
    );
    litMaterials[0].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\flashlight\\Flashlight_Roughness.dds"
    );

    litMaterials[1].Albedo = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\flashlight\\Flashlight_Base_color.dds"
    );
    litMaterials[1].Normal = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\flashlight\\Flashlight_Normal.dds"
    );
    litMaterials[1].Metallic = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\flashlight\\Flashlight_Metallic.dds"
    );
    litMaterials[1].Roughness = TextureManager::LoadTexture2D(
        Filesystem::GetTextureDir() + L"models\\flashlight\\Flashlight_Roughness.dds"
    );

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
