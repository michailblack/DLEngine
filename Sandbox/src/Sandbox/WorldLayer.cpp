#include "WorldLayer.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/Math/Math.h"

#include "DLEngine/Renderer/Renderer.h"

#include <imgui/imgui.h>

void WorldLayer::OnAttach()
{
    const auto& windowSize{ DLEngine::Application::Get().GetWindow().GetSize() };

    DLEngine::TextureSpecification skyboxSpecification{};
    skyboxSpecification.DebugName = "Skybox Night Street";
    skyboxSpecification.Usage = DLEngine::TextureUsage::Texture;
    const auto& skybox{ DLEngine::Renderer::GetTextureLibrary()->LoadTextureCube(
        skyboxSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "skybox\\night_street.dds"
    ) };

    DLEngine::SceneSpecification sceneSpecification{};
    sceneSpecification.CameraResizeCallback = [](DLEngine::Camera& camera, uint32_t width, uint32_t height)
        {
            const float aspectRatio{ static_cast<float>(width) / static_cast<float>(height) };
            camera.SetPerspectiveProjectionFov(DLEngine::Math::ToRadians(60.0f), aspectRatio, 20.0f, 0.001f);
        };
    sceneSpecification.ViewportWidth = static_cast<uint32_t>(windowSize.x);
    sceneSpecification.ViewportHeight = static_cast<uint32_t>(windowSize.y);
    m_Scene = DLEngine::CreateRef<DLEngine::Scene>(sceneSpecification);

    DLEngine::SceneRendererSpecification sceneRendererSpecification{};
    sceneRendererSpecification.Skybox = skybox;
    sceneRendererSpecification.ViewportWidth = sceneSpecification.ViewportWidth;
    sceneRendererSpecification.ViewportHeight = sceneSpecification.ViewportHeight;
    m_SceneRenderer = DLEngine::CreateRef<DLEngine::SceneRenderer>(sceneRendererSpecification);

    m_PostProcessSettings.EV100 = -2.0f;

    AddObjectsToScene();
}

void WorldLayer::OnDetach()
{

}

void WorldLayer::OnUpdate(DeltaTime dt)
{
    m_Scene->OnUpdate(dt);
    
    if (m_IsFlashlightAttached)
    {
        const auto& sceneCamera{ m_Scene->GetCamera() };
        auto newFlashlightTransform{ m_FlashlightBaseTransform *
            DLEngine::Math::Mat4x4::Inverse(sceneCamera.GetViewMatrix()) *
            DLEngine::Math::Mat4x4::Translate(sceneCamera.GetForward() * 0.4f) *
            DLEngine::Math::Mat4x4::Translate(sceneCamera.GetRight() * 0.2f) *
            DLEngine::Math::Mat4x4::Translate(sceneCamera.GetUp() * -0.15f)
        };
        
        m_FlashlightInstance->Set("TRANSFORM", DLEngine::Buffer{ &newFlashlightTransform, sizeof(DLEngine::Math::Mat4x4) });
    }

    m_SceneRenderer->RenderScene(m_Scene);
}

void WorldLayer::OnImGuiRender()
{
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

    if (ImGui::CollapsingHeader("PBR"))
    {
        ImGui::Checkbox("Use IBL", reinterpret_cast<bool*>(&m_PBRSettings.UseIBL));

        if (!m_PBRSettings.UseIBL)
        {
            ImGui::Text("Indirect Light Radiance");
            ImGui::DragFloat3("##indirect_light_radiance", &m_PBRSettings.IndirectLightRadiance.x, 0.01f, 0.0f, 1.0f);
        }

        ImGui::Checkbox("Use Diffuse Reflections", reinterpret_cast<bool*>(&m_PBRSettings.UseDiffuseReflections));
        ImGui::Checkbox("Use Specular Reflections", reinterpret_cast<bool*>(&m_PBRSettings.UseSpecularReflections));

        ImGui::Checkbox("Overwrite Roughness", reinterpret_cast<bool*>(&m_PBRSettings.OverwriteRoughness));
            
        if (m_PBRSettings.OverwriteRoughness)
        {
            ImGui::Text("Overwritten Roughness");
            ImGui::SliderFloat("##overwritten_roughness", &m_PBRSettings.OverwrittenRoughness, 0.0f, 1.0f);
        }
    }

    if (ImGui::CollapsingHeader("Shadow Mapping"))
    {
        static constexpr int32_t mapSizes[]{ 512, 1024, 2048, 4096 };
        static const char* mapSizeNames[]{ "512", "1024", "2048", "4096" };
        static int32_t index{ 2 };

        ImGui::Text("Map Size");
        if (ImGui::Combo("##map_size", &index, mapSizeNames, IM_ARRAYSIZE(mapSizeNames)))
            m_ShadowMapSettings.MapSize = static_cast<uint32_t>(mapSizes[index]);

        ImGui::Checkbox("Use Directional Shadows", &m_ShadowMapSettings.UseDirectionalShadows);

        if (m_ShadowMapSettings.UseDirectionalShadows)
        {
            ImGui::Text("Directional Light Shadow Margin");
            ImGui::SliderFloat("##directional_light_shadow_margin", &m_ShadowMapSettings.DirectionalLightShadowMargin, 0.0f, 50.0f);

            ImGui::Text("Directional Light Shadow Distance");
            ImGui::SliderFloat("##directional_light_shadow_distance", &m_ShadowMapSettings.DirectionalLightShadowDistance, 0.0f, 50.0f);
        }
        
        ImGui::Checkbox("Use Omnidirectional Shadows", &m_ShadowMapSettings.UseOmnidirectionalShadows);
        ImGui::Checkbox("Use Spot Shadows", &m_ShadowMapSettings.UseSpotShadows);
        ImGui::Checkbox("Use PCF", &m_ShadowMapSettings.UsePCF);
    }

    if (ImGui::CollapsingHeader("Post Processing"))
    {
        ImGui::Text("EV100");
        ImGui::SliderFloat("##ev100", &m_PostProcessSettings.EV100, -10.0f, 10.0f);

        ImGui::Text("Gamma");
        ImGui::SliderFloat("##gamma", &m_PostProcessSettings.Gamma, 0.1f, 5.0f);
    }

    ImGui::End();

    m_SceneRenderer->SetPBRSettings(m_PBRSettings);
    m_SceneRenderer->SetPostProcessSettings(m_PostProcessSettings);
    m_SceneRenderer->SetShadowMapSettings(m_ShadowMapSettings);
}

void WorldLayer::OnEvent(DLEngine::Event& e)
{
    DLEngine::EventDispatcher dispatcher{ e };
    dispatcher.Dispatch<DLEngine::KeyPressedEvent>(DL_BIND_EVENT_FN(WorldLayer::OnKeyPressedEvent));

    m_Scene->OnEvent(e);
}

void WorldLayer::AddObjectsToScene()
{
    const auto& cube{ DLEngine::Renderer::GetMeshLibrary()->Load(DLEngine::Mesh::GetMeshDirectoryPath() / "cube\\cube.obj") };
    const auto& samurai{ DLEngine::Renderer::GetMeshLibrary()->Load(DLEngine::Mesh::GetMeshDirectoryPath() / "samurai\\samurai.fbx") };
    const auto& flashlight{ DLEngine::Renderer::GetMeshLibrary()->Load(DLEngine::Mesh::GetMeshDirectoryPath() / "flashlight\\flashlight.fbx") };
    const auto& pbrStaticShader{ DLEngine::Renderer::GetShaderLibrary()->Get("PBR_Static") };
    auto pbrMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Cube Steel Material") };

    // Spawning cubes
    {
        DLEngine::TextureSpecification textureSpecification{};
        textureSpecification.Usage = DLEngine::TextureUsage::Texture;

        textureSpecification.DebugName = "Cobblestone Albedo";
        const auto& cobblestoneAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\cube\\cobblestone\\Cobblestone_albedo.dds") };

        textureSpecification.DebugName = "Cobblestone Normal";
        const auto& cobblestoneNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\cube\\cobblestone\\Cobblestone_normal.dds") };

        auto steelPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial steelCBPBRMaterial{};
        steelCBPBRMaterial.UseNormalMap = true;
        steelCBPBRMaterial.FlipNormalMapY = false;
        steelCBPBRMaterial.HasMetalnessMap = false;
        steelCBPBRMaterial.DefaultMetalness = 0.0f;
        steelCBPBRMaterial.HasRoughnessMap = false;
        steelCBPBRMaterial.DefaultRoughness = 1.0f;
        steelPBRCB->SetData(DLEngine::Buffer{ &steelCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        pbrMaterial->Set("t_Albedo", cobblestoneAlbedo);
        pbrMaterial->Set("t_Normal", cobblestoneNormal);
        pbrMaterial->Set("PBRMaterial", steelPBRCB);

#if 0
        const auto transform{ DLEngine::Math::Mat4x4::Scale(DLEngine::Math::Vec3{ 10.0f, 0.5f, 10.0f }) *
            DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 0.0f, -0.5f, 0.0f })
        };
        auto instance{ DLEngine::Instance::Create(pbrStaticShader, "PBR_Static Cube Instance") };
        instance->Set("TRANSFORM", DLEngine::Buffer{ &transform, sizeof(DLEngine::Math::Mat4x4) });

        m_Scene->AddSubmesh(cube, 0u, pbrMaterial, instance);
#endif

        for (int32_t x{ -5 }; x < 5; ++x)
        {
            for (int32_t z{ -5 }; z < 5; ++z)
            {
                const DLEngine::Math::Mat4x4 transform{ DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ static_cast<float>(x), -1.0f, static_cast<float>(z) }) };
                const auto& instance{ DLEngine::Instance::Create(pbrStaticShader, "PBR_Static Cube Instance") };
                instance->Set("TRANSFORM", DLEngine::Buffer{ &transform, sizeof(DLEngine::Math::Mat4x4) });

                m_Scene->AddSubmesh(cube, 0u, pbrMaterial, instance);
            }
        }
    }

    // Spawning samurais
    {
        const auto baseTransform{ DLEngine::Math::Mat4x4::Rotate(DLEngine::Math::ToRadians(-90.0f), 0.0f, 0.0f) };
        std::vector<DLEngine::Math::Mat4x4> samuraiTransforms{};
        samuraiTransforms.emplace_back(baseTransform * DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -1.5f, 0.0f, 2.0f }));
        samuraiTransforms.emplace_back(baseTransform * DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 1.5f, 0.0f, 2.0f }));

        std::vector<DLEngine::Ref<DLEngine::Instance>> samuraiInstances{};
        samuraiInstances.reserve(samuraiTransforms.size());
        for (const auto& transform : samuraiTransforms)
        {
            auto samuraiInstance{ DLEngine::Instance::Create(pbrStaticShader, "PBR_Static Samurai Instance") };
            samuraiInstance->Set("TRANSFORM", DLEngine::Buffer{ &transform, sizeof(DLEngine::Math::Mat4x4) });
            samuraiInstances.emplace_back(samuraiInstance);
        }

        DLEngine::TextureSpecification textureSpecification{};
        textureSpecification.Usage = DLEngine::TextureUsage::Texture;

        textureSpecification.DebugName = "Sword Albedo";
        const auto& swordAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Sword_BaseColor.dds") };

        textureSpecification.DebugName = "Sword Normal";
        const auto& swordNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Sword_Normal.dds") };

        textureSpecification.DebugName = "Sword Metalness";
        const auto& swordMetalness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Sword_Metallic.dds") };

        textureSpecification.DebugName = "Sword Roughness";
        const auto& swordRoughness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Sword_Roughness.dds") };

        auto swordPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial swordCBPBRMaterial{};
        swordCBPBRMaterial.UseNormalMap = true;
        swordCBPBRMaterial.FlipNormalMapY = false;
        swordCBPBRMaterial.HasMetalnessMap = true;
        swordCBPBRMaterial.HasRoughnessMap = true;
        swordPBRCB->SetData(DLEngine::Buffer{ &swordCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        auto swordMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Samurai Sword Material") };
        swordMaterial->Set("t_Albedo", swordAlbedo);
        swordMaterial->Set("t_Normal", swordNormal);
        swordMaterial->Set("t_Metalness", swordMetalness);
        swordMaterial->Set("t_Roughness", swordRoughness);
        swordMaterial->Set("PBRMaterial", swordPBRCB);

        for (uint32_t instanceIndex{ 0u }; instanceIndex < samuraiInstances.size(); ++instanceIndex)
            m_Scene->AddSubmesh(samurai, 0u, swordMaterial, samuraiInstances[instanceIndex]);

        textureSpecification.DebugName = "Head Albedo";
        const auto& headAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Head_BaseColor.dds") };

        textureSpecification.DebugName = "Head Normal";
        const auto& headNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Head_Normal.dds") };

        textureSpecification.DebugName = "Head Roughness";
        const auto& headRoughness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Head_Roughness.dds") };

        auto headPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial headCBPBRMaterial{};
        headCBPBRMaterial.UseNormalMap = true;
        headCBPBRMaterial.FlipNormalMapY = false;
        headCBPBRMaterial.HasMetalnessMap = false;
        headCBPBRMaterial.DefaultMetalness = 0.0f;
        headCBPBRMaterial.HasRoughnessMap = true;
        headPBRCB->SetData(DLEngine::Buffer{ &headCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        auto headMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Samurai Head Material") };
        headMaterial->Set("t_Albedo", headAlbedo);
        headMaterial->Set("t_Normal", headNormal);
        headMaterial->Set("t_Roughness", headRoughness);
        headMaterial->Set("PBRMaterial", headPBRCB);

        for (uint32_t instanceIndex{ 0u }; instanceIndex < samuraiInstances.size(); ++instanceIndex)
            m_Scene->AddSubmesh(samurai, 1u, headMaterial, samuraiInstances[instanceIndex]);

        textureSpecification.DebugName = "Eyes Albedo";
        const auto& eyesAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Eyes_BaseColor.dds") };

        textureSpecification.DebugName = "Eyes Normal";
        const auto& eyesNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Eyes_Normal.dds") };

        auto eyesPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial eyesCBPBRMaterial{};
        eyesCBPBRMaterial.UseNormalMap = true;
        eyesCBPBRMaterial.FlipNormalMapY = false;
        eyesCBPBRMaterial.HasMetalnessMap = false;
        eyesCBPBRMaterial.DefaultMetalness = 0.0f;
        eyesCBPBRMaterial.HasRoughnessMap = false;
        eyesCBPBRMaterial.DefaultRoughness = 0.0f;
        eyesPBRCB->SetData(DLEngine::Buffer{ &eyesCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        auto eyesMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Samurai Eyes Material") };
        eyesMaterial->Set("t_Albedo", eyesAlbedo);
        eyesMaterial->Set("t_Normal", eyesNormal);
        eyesMaterial->Set("PBRMaterial", eyesPBRCB);

        for (uint32_t instanceIndex{ 0u }; instanceIndex < samuraiInstances.size(); ++instanceIndex)
            m_Scene->AddSubmesh(samurai, 2u, eyesMaterial, samuraiInstances[instanceIndex]);

        textureSpecification.DebugName = "Helmet Albedo";
        const auto& helmetAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Helmet_BaseColor.dds") };

        textureSpecification.DebugName = "Helmet Normal";
        const auto& helmetNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Helmet_Normal.dds") };

        textureSpecification.DebugName = "Helmet Metalness";
        const auto& helmetMetalness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Helmet_Metallic.dds") };

        textureSpecification.DebugName = "Helmet Roughness";
        const auto& helmetRoughness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Helmet_Roughness.dds") };

        auto helmetPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial helmetCBPBRMaterial{};
        helmetCBPBRMaterial.UseNormalMap = true;
        helmetCBPBRMaterial.FlipNormalMapY = false;
        helmetCBPBRMaterial.HasMetalnessMap = true;
        helmetCBPBRMaterial.HasRoughnessMap = true;
        helmetPBRCB->SetData(DLEngine::Buffer{ &helmetCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        auto helmetMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Samurai Helmet Material") };
        helmetMaterial->Set("t_Albedo", helmetAlbedo);
        helmetMaterial->Set("t_Normal", helmetNormal);
        helmetMaterial->Set("t_Metalness", helmetMetalness);
        helmetMaterial->Set("t_Roughness", helmetRoughness);
        helmetMaterial->Set("PBRMaterial", helmetPBRCB);

        for (uint32_t instanceIndex{ 0u }; instanceIndex < samuraiInstances.size(); ++instanceIndex)
            m_Scene->AddSubmesh(samurai, 3u, helmetMaterial, samuraiInstances[instanceIndex]);

        textureSpecification.DebugName = "Decor Albedo";
        const auto& decorAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Decor_BaseColor.dds") };

        textureSpecification.DebugName = "Decor Normal";
        const auto& decorNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Decor_Normal.dds") };

        textureSpecification.DebugName = "Decor Metalness";
        const auto& decorMetalness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Decor_Metallic.dds") };

        textureSpecification.DebugName = "Decor Roughness";
        const auto& decorRoughness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Decor_Roughness.dds") };

        auto decorPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial decorCBPBRMaterial{};
        decorCBPBRMaterial.UseNormalMap = true;
        decorCBPBRMaterial.FlipNormalMapY = false;
        decorCBPBRMaterial.HasMetalnessMap = true;
        decorCBPBRMaterial.HasRoughnessMap = true;
        decorPBRCB->SetData(DLEngine::Buffer{ &decorCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        auto decorMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Samurai Decor Material") };
        decorMaterial->Set("t_Albedo", decorAlbedo);
        decorMaterial->Set("t_Normal", decorNormal);
        decorMaterial->Set("t_Metalness", decorMetalness);
        decorMaterial->Set("t_Roughness", decorRoughness);
        decorMaterial->Set("PBRMaterial", decorPBRCB);

        for (uint32_t instanceIndex{ 0u }; instanceIndex < samuraiInstances.size(); ++instanceIndex)
            m_Scene->AddSubmesh(samurai, 4u, decorMaterial, samuraiInstances[instanceIndex]);

        textureSpecification.DebugName = "Pants Albedo";
        const auto& pantsAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Pants_BaseColor.dds") };

        textureSpecification.DebugName = "Pants Normal";
        const auto& pantsNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Pants_Normal.dds") };

        textureSpecification.DebugName = "Pants Metalness";
        const auto& pantsMetalness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Pants_Metallic.dds") };

        textureSpecification.DebugName = "Pants Roughness";
        const auto& pantsRoughness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Pants_Roughness.dds") };

        auto pantsPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial pantsCBPBRMaterial{};
        pantsCBPBRMaterial.UseNormalMap = true;
        pantsCBPBRMaterial.FlipNormalMapY = false;
        pantsCBPBRMaterial.HasMetalnessMap = true;
        pantsCBPBRMaterial.HasRoughnessMap = true;
        pantsPBRCB->SetData(DLEngine::Buffer{ &pantsCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        auto pantsMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Samurai Pants Material") };
        pantsMaterial->Set("t_Albedo", pantsAlbedo);
        pantsMaterial->Set("t_Normal", pantsNormal);
        pantsMaterial->Set("t_Metalness", pantsMetalness);
        pantsMaterial->Set("t_Roughness", pantsRoughness);
        pantsMaterial->Set("PBRMaterial", pantsPBRCB);

        for (uint32_t instanceIndex{ 0u }; instanceIndex < samuraiInstances.size(); ++instanceIndex)
            m_Scene->AddSubmesh(samurai, 5u, pantsMaterial, samuraiInstances[instanceIndex]);

        textureSpecification.DebugName = "Hands Albedo";
        const auto& handsAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Hands_BaseColor.dds") };

        textureSpecification.DebugName = "Hands Normal";
        const auto& handsNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Hands_Normal.dds") };

        textureSpecification.DebugName = "Hands Roughness";
        const auto& handsRoughness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Hands_Roughness.dds") };

        auto handsPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial handsCBPBRMaterial{};
        handsCBPBRMaterial.UseNormalMap = true;
        handsCBPBRMaterial.FlipNormalMapY = false;
        handsCBPBRMaterial.HasMetalnessMap = false;
        handsCBPBRMaterial.DefaultMetalness = 0.0f;
        handsCBPBRMaterial.HasRoughnessMap = true;
        handsPBRCB->SetData(DLEngine::Buffer{ &handsCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        auto handsMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Samurai Hands Material") };
        handsMaterial->Set("t_Albedo", handsAlbedo);
        handsMaterial->Set("t_Normal", handsNormal);
        handsMaterial->Set("t_Roughness", handsRoughness);
        handsMaterial->Set("PBRMaterial", handsPBRCB);

        for (uint32_t instanceIndex{ 0u }; instanceIndex < samuraiInstances.size(); ++instanceIndex)
            m_Scene->AddSubmesh(samurai, 6u, handsMaterial, samuraiInstances[instanceIndex]);

        textureSpecification.DebugName = "Torso Albedo";
        const auto& torsoAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Torso_BaseColor.dds") };

        textureSpecification.DebugName = "Torso Normal";
        const auto& torsoNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Torso_Normal.dds") };

        textureSpecification.DebugName = "Torso Metalness";
        const auto& torsoMetalness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Torso_Metallic.dds") };

        textureSpecification.DebugName = "Torso Roughness";
        const auto& torsoRoughness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\samurai\\Torso_Roughness.dds") };

        auto torsoPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial torsoCBPBRMaterial{};
        torsoCBPBRMaterial.UseNormalMap = true;
        torsoCBPBRMaterial.FlipNormalMapY = false;
        torsoCBPBRMaterial.HasMetalnessMap = true;
        torsoCBPBRMaterial.HasRoughnessMap = true;
        torsoPBRCB->SetData(DLEngine::Buffer{ &torsoCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        auto torsoMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Samurai Torso Material") };
        torsoMaterial->Set("t_Albedo", torsoAlbedo);
        torsoMaterial->Set("t_Normal", torsoNormal);
        torsoMaterial->Set("t_Metalness", torsoMetalness);
        torsoMaterial->Set("t_Roughness", torsoRoughness);
        torsoMaterial->Set("PBRMaterial", torsoPBRCB);

        for (uint32_t instanceIndex{ 0u }; instanceIndex < samuraiInstances.size(); ++instanceIndex)
            m_Scene->AddSubmesh(samurai, 7u, torsoMaterial, samuraiInstances[instanceIndex]);
    }

    // Adding lights
    {
        m_Scene->AddDirectionalLight(DLEngine::Math::Normalize(DLEngine::Math::Vec3{ -1.0f, -1.0f, 1.0f }), DLEngine::Math::Vec3{ 3.66e+3f, 3.66e+3f, 4.08e+3f }, 6.8e-5f);
        m_Scene->AddPointLight(DLEngine::Math::Vec3{ 0.0f }, DLEngine::Math::Vec3{ 1.0f }, 0.1f, 1.0f, DLEngine::Math::Vec3{ -1.5f, 3.0f, 1.0f });
        m_Scene->AddPointLight(DLEngine::Math::Vec3{ 0.0f }, DLEngine::Math::Vec3{ 1.0f }, 0.3f, 1.0f, DLEngine::Math::Vec3{ 1.5f, 3.0f, 1.0f });
        m_Scene->AddPointLight(DLEngine::Math::Vec3{ 0.0f }, DLEngine::Math::Vec3{ 2.0f, 15.0f, 4.0f }, 0.3f, 0.7f, DLEngine::Math::Vec3{ 0.0f, 4.0f, 2.0f });

        m_FlashlightBaseTransform = DLEngine::Math::Mat4x4::Scale(DLEngine::Math::Vec3{ 0.003f }) *
            DLEngine::Math::Mat4x4::Rotate(DLEngine::Math::ToRadians(-90.0f), DLEngine::Math::ToRadians(180.0f), 0.0f);
        
        DLEngine::TextureSpecification textureSpecification{};
        textureSpecification.Usage = DLEngine::TextureUsage::Texture;

        textureSpecification.DebugName = "Flashlight Albedo";
        const auto& flashlightAlbedo{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\flashlight\\Flashlight_Base_color.dds") };

        textureSpecification.DebugName = "Flashlight Normal";
        const auto& flashlightNormal{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\flashlight\\Flashlight_Normal.dds") };

        textureSpecification.DebugName = "Flashlight Metalness";
        const auto& flashlightMetalness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\flashlight\\Flashlight_Metallic.dds") };

        textureSpecification.DebugName = "Flashlight Roughness";
        const auto& flashlightRoughness{ DLEngine::Texture2D::Create(textureSpecification, DLEngine::Texture::GetTextureDirectoryPath() / "models\\flashlight\\Flashlight_Roughness.dds") };

        auto flashlightPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial flashlightCBPBRMaterial{};
        flashlightCBPBRMaterial.UseNormalMap = true;
        flashlightCBPBRMaterial.FlipNormalMapY = false;
        flashlightCBPBRMaterial.HasMetalnessMap = true;
        flashlightCBPBRMaterial.HasRoughnessMap = true;
        flashlightPBRCB->SetData(DLEngine::Buffer{ &flashlightCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        auto flashlightMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Flashlight Material") };
        flashlightMaterial->Set("t_Albedo", flashlightAlbedo);
        flashlightMaterial->Set("t_Normal", flashlightNormal);
        flashlightMaterial->Set("t_Metalness", flashlightMetalness);
        flashlightMaterial->Set("t_Roughness", flashlightRoughness);
        flashlightMaterial->Set("PBRMaterial", flashlightPBRCB);

        m_FlashlightInstance = DLEngine::Instance::Create(pbrStaticShader, "PBR_Static Flashlight Instance");
        m_FlashlightInstance->Set("TRANSFORM", DLEngine::Buffer{ &m_FlashlightBaseTransform, sizeof(DLEngine::Math::Mat4x4) });

        m_Scene->AddSubmesh(flashlight, 0u, flashlightMaterial, m_FlashlightInstance);

        m_Scene->AddSpotLight(
            DLEngine::Math::Vec3{ 0.0f },
            DLEngine::Math::Vec3{ 0.0f, -1.0f, 0.0f },
            0.075f,
            DLEngine::Math::Cos(DLEngine::Math::ToRadians(5.0f)),
            DLEngine::Math::Cos(DLEngine::Math::ToRadians(7.5f)),
            DLEngine::Math::Vec3{ 4.0f, 1.0f, 15.0f },
            2.0f,
            m_FlashlightInstance
        );
    }
}

bool WorldLayer::OnKeyPressedEvent(DLEngine::KeyPressedEvent& e)
{
    if (e.GetKeyCode() == 'F')
        m_IsFlashlightAttached = !m_IsFlashlightAttached;

    return false;
}
