#include "WorldLayer.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/Math/Math.h"

#include "DLEngine/Renderer/Renderer.h"

#include "DLEngine/Utils/RandomGenerator.h"

#include <imgui/imgui.h>

#include <unordered_set>

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

    m_PostProcessingSettings.EV100 = -0.5f;

    m_SmokeEmitterToSpawn.SpawnedParticleTintColor = DLEngine::Math::Vec3{ 0.5f, 0.5f, 0.5f };
    m_SmokeEmitterToSpawn.ParticleEmissionIntensity = 2.0f;
    m_SmokeEmitterToSpawn.InitialParticleSize = DLEngine::Math::Vec2{ 0.1f, 0.1f };
    m_SmokeEmitterToSpawn.FinalParticleSize = DLEngine::Math::Vec2{ 0.3f, 0.3f };
    m_SmokeEmitterToSpawn.ParticleSpawnRadius = 0.1f;
    m_SmokeEmitterToSpawn.MinParticleLifetimeMS = 100.0f;
    m_SmokeEmitterToSpawn.MaxParticleLifetimeMS = 1000.0f;
    m_SmokeEmitterToSpawn.ParticleVerticalVelocity = 0.2f;
    m_SmokeEmitterToSpawn.ParticleSpawnRatePerSecond = 5000u;

    LoadMeshes();
    LoadTextures();
    AddObjectsToScene();
}

void WorldLayer::OnDetach()
{

}

void WorldLayer::OnUpdate(DLEngine::DeltaTime dt)
{
    DLEngine::DeltaTime scaledDeltaTime{ dt * m_TimeScale };

    m_Time += scaledDeltaTime.GetSeconds();
    m_DeltaTime = scaledDeltaTime;

    m_Scene->OnUpdate(scaledDeltaTime);
    
    if (m_IsFlashlightAttached)
    {
        const auto& sceneCamera{ m_Scene->GetCamera() };
        auto newFlashlightTransform{ m_FlashlightBaseTransform *
            DLEngine::Math::Mat4x4::Inverse(sceneCamera.GetViewMatrix()) *
            DLEngine::Math::Mat4x4::Translate(sceneCamera.GetForward() * 0.4f) *
            DLEngine::Math::Mat4x4::Translate(sceneCamera.GetRight() * 0.2f) *
            DLEngine::Math::Mat4x4::Translate(sceneCamera.GetUp() * -0.15f)
        };
        
        m_Scene->GetMeshRegistry().GetInstance(m_FlashlightMeshUUID)->Set("TRANSFORM", DLEngine::Buffer{ &newFlashlightTransform, sizeof(DLEngine::Math::Mat4x4) });
    }

    SwapDissolutionGroupInstances(scaledDeltaTime);

    m_SceneRenderer->RenderScene(m_Scene);
}

void WorldLayer::OnImGuiRender()
{
    const ImVec2 mainWindowSize{ ImGui::GetMainViewport()->Size };

    constexpr float relativeWidth{ 0.25f };

    static float s_PrevWidth{ mainWindowSize.x * relativeWidth };

    const ImVec2 minWindowSize{ mainWindowSize.x * relativeWidth, mainWindowSize.y };
    const ImVec2 windowPos{ mainWindowSize.x - s_PrevWidth, 0.0f };

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(minWindowSize, ImVec2{ mainWindowSize.x, minWindowSize.y });

    constexpr ImGuiWindowFlags windowFlags{ ImGuiWindowFlags_NoMove };

    ImGui::Begin("Scene", nullptr, windowFlags);

    const ImVec2 windowSize{ ImGui::GetWindowSize() };
    s_PrevWidth = windowSize.x;

    ImGui::Text("Time Scale");
    ImGui::SliderFloat("##time_scale", &m_TimeScale, 0.0f, 5.0f);

    if (ImGui::CollapsingHeader("Statistics"))
    {
        ImGui::Text(std::format("Time (s): {0:.2f}", m_Time).c_str());
        ImGui::Text(std::format("Delta time (ms): {0:.2f}", m_DeltaTime).c_str());
        ImGui::Text(std::format("Overall Particles Count: {0}", m_Scene->GetOverallParticlesCount()).c_str());
    }

    if (ImGui::CollapsingHeader("Settings"))
    {
        ImGui::Indent();

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
                m_ShadowMappingSettings.MapSize = static_cast<uint32_t>(mapSizes[index]);

            ImGui::Checkbox("Use Directional Shadows", &m_ShadowMappingSettings.UseDirectionalShadows);

            if (m_ShadowMappingSettings.UseDirectionalShadows)
            {
                ImGui::Text("Directional Light Shadow Margin");
                ImGui::SliderFloat("##directional_light_shadow_margin", &m_ShadowMappingSettings.DirectionalLightShadowMargin, 0.0f, 50.0f);

                ImGui::Text("Directional Light Shadow Distance");
                ImGui::SliderFloat("##directional_light_shadow_distance", &m_ShadowMappingSettings.DirectionalLightShadowDistance, 0.0f, 50.0f);
            }

            ImGui::Checkbox("Use Omnidirectional Shadows", &m_ShadowMappingSettings.UseOmnidirectionalShadows);
            ImGui::Checkbox("Use Spot Shadows", &m_ShadowMappingSettings.UseSpotShadows);
            ImGui::Checkbox("Use PCF", &m_ShadowMappingSettings.UsePCF);

            ImGui::Text("Shadow Bias");
            ImGui::SliderFloat("##shadow_bias", &m_ShadowMappingSettings.ShadowBias, 0.0f, 0.1f);
        }

        if (ImGui::CollapsingHeader("Post Processing"))
        {
            ImGui::Text("EV100");
            ImGui::SliderFloat("##ev100", &m_PostProcessingSettings.EV100, -10.0f, 10.0f);

            ImGui::Text("Gamma");
            ImGui::SliderFloat("##gamma", &m_PostProcessingSettings.Gamma, 0.1f, 5.0f);

            ImGui::Text("FXAA Quality Sub-pixel");
            ImGui::SliderFloat("##fxaa_quality_subpix", &m_PostProcessingSettings.FXAA_QualitySubpix, 0.0f, 1.0f);

            ImGui::Text("FXAA Quality Edge Threshold");
            ImGui::SliderFloat("##fxaa_quality_edge_threshold", &m_PostProcessingSettings.FXAA_QualityEdgeThreshold, 0.063f, 0.333f);

            ImGui::Text("FXAA Quality Edge Threshold Min");
            ImGui::SliderFloat("##fxaa_quality_edge_threshold_min", &m_PostProcessingSettings.FXAA_QualityEdgeThresholdMin, 0.0312f, 0.0833f);
        }

        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Dissolution Group Spawning"))
    {
        ImGui::Text("Distance To Camera");
        ImGui::SliderFloat("##distance_to_camera", &m_DissolutionGroupSpawnSettings.DistanceToCamera, 0.0f, 10.0f);

        ImGui::Text("Min Dissolution Duration (s)");
        ImGui::SliderFloat("##min_dissolution_duration", &m_DissolutionGroupSpawnSettings.MinDissolutionDuration, 0.0f, 10.0f);

        m_DissolutionGroupSpawnSettings.MaxDissolutionDuration = DLEngine::Math::Clamp(
            m_DissolutionGroupSpawnSettings.MaxDissolutionDuration,
            m_DissolutionGroupSpawnSettings.MinDissolutionDuration,
            m_DissolutionGroupSpawnSettings.MinDissolutionDuration + 10.0f
        );

        ImGui::Text("Max Dissolution Duration (s)");
        ImGui::SliderFloat("##max_dissolution_duration", &m_DissolutionGroupSpawnSettings.MaxDissolutionDuration, m_DissolutionGroupSpawnSettings.MinDissolutionDuration, m_DissolutionGroupSpawnSettings.MinDissolutionDuration + 10.0f);
    }

    if (ImGui::CollapsingHeader("Smoke Emitter Spawning"))
    {
        ImGui::Text("Particle Tint Color");
        ImGui::ColorEdit3("##smoke_emitter_particle_color", &m_SmokeEmitterToSpawn.SpawnedParticleTintColor.x);

        ImGui::Text("Particle Emission Intensity");
        ImGui::SliderFloat("##smoke_emitter_particle_emission_intensity", &m_SmokeEmitterToSpawn.ParticleEmissionIntensity, 1.0f, 5.0f);

        ImGui::Text("Initial Particle Size");
        ImGui::DragFloat2("##smoke_emitter_initial_particle_size", &m_SmokeEmitterToSpawn.InitialParticleSize.x, 0.001f, 0.0f, 2.0f, "%.5f");

        ImGui::Text("Final Particle Size");
        ImGui::DragFloat2("##smoke_emitter_final_particle_size", &m_SmokeEmitterToSpawn.FinalParticleSize.x, 0.001f, 0.0f, 2.0f, "%.5f");

        ImGui::Text("Particle Spawn Radius");
        ImGui::SliderFloat("##smoke_emitter_particle_spawn_radius", &m_SmokeEmitterToSpawn.ParticleSpawnRadius, 0.0f, 2.0f, "%.5f");

        constexpr float lifetimeDuration{ 5000.0f };
        ImGui::Text("Min Particle Lifetime (ms)");
        ImGui::SliderFloat("##smoke_emitter_min_particle_lifetime", &m_SmokeEmitterToSpawn.MinParticleLifetimeMS, 0.0f, lifetimeDuration);

        m_SmokeEmitterToSpawn.MaxParticleLifetimeMS = DLEngine::Math::Clamp(
            m_SmokeEmitterToSpawn.MaxParticleLifetimeMS,
            m_SmokeEmitterToSpawn.MinParticleLifetimeMS,
            m_SmokeEmitterToSpawn.MinParticleLifetimeMS + lifetimeDuration
        );

        ImGui::Text("Max Particle Lifetime (ms)");
        ImGui::SliderFloat("##smoke_emitter_max_particle_lifetime", &m_SmokeEmitterToSpawn.MaxParticleLifetimeMS, m_SmokeEmitterToSpawn.MinParticleLifetimeMS, m_SmokeEmitterToSpawn.MinParticleLifetimeMS + lifetimeDuration);

        ImGui::Text("Particle Vertical Velocity Per Second");
        ImGui::SliderFloat("##smoke_emitter_particle_vertical_velocity", &m_SmokeEmitterToSpawn.ParticleVerticalVelocity, 0.0f, 2.0f, "%.5f");

        ImGui::Text("Particle Spawn Rate Per Second");
        ImGui::SliderInt("##smoke_emitter_particle_spawn_rate_per_second", reinterpret_cast<int32_t*>(&m_SmokeEmitterToSpawn.ParticleSpawnRatePerSecond), 1, 10000);

        ImGui::Text("Emitter Distance To Camera");
        ImGui::SliderFloat("##emitter_distance_to_camera", &m_SmokeEmitterSpawnDistanceToCamera, 0.0f, 10.0f);

        m_SmokeEmitterToSpawn.ParticleHorizontalVelocity = m_SmokeEmitterToSpawn.ParticleVerticalVelocity;

        const auto& sceneCameraPosition{ m_Scene->GetCamera().GetPosition() };
        const auto& sceneCameraForward{ m_Scene->GetCamera().GetForward() };

        if (ImGui::Button("Spawn Smoke Emitter"))
            m_Scene->AddSmokeEmitter(m_SmokeEmitterToSpawn, sceneCameraPosition + m_SmokeEmitterSpawnDistanceToCamera * sceneCameraForward);

        if (ImGui::Button("Clear Smoke Emitters"))
            m_Scene->ClearSmokeEmitters();
    }

    ImGui::End();

    m_SceneRenderer->SetPBRSettings(m_PBRSettings);
    m_SceneRenderer->SetPostProcessingSettings(m_PostProcessingSettings);
    m_SceneRenderer->SetShadowMappingSettings(m_ShadowMappingSettings);
}

void WorldLayer::OnEvent(DLEngine::Event& e)
{
    DLEngine::EventDispatcher dispatcher{ e };
    dispatcher.Dispatch<DLEngine::KeyPressedEvent>(DL_BIND_EVENT_FN(WorldLayer::OnKeyPressedEvent));

    m_Scene->OnEvent(e);
}

void WorldLayer::LoadMeshes()
{
    auto meshLibrary{ DLEngine::Renderer::GetMeshLibrary() };
    const auto& meshDirectoryPath{ DLEngine::Mesh::GetMeshDirectoryPath() };

    meshLibrary->Load(meshDirectoryPath / "samurai\\samurai.fbx");
    meshLibrary->Load(meshDirectoryPath / "cube\\cube.obj");
    meshLibrary->Load(meshDirectoryPath / "flashlight\\flashlight.fbx");
}

void WorldLayer::LoadTextures()
{
    auto textureLibrary{ DLEngine::Renderer::GetTextureLibrary() };
    const auto& textureDirectoryPath{ DLEngine::Texture::GetTextureDirectoryPath() };

    DLEngine::TextureSpecification textureSpecification{};
    textureSpecification.Usage = DLEngine::TextureUsage::Texture;

    // Cube textures
    textureSpecification.DebugName = "Cobblestone Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\cobblestone\\Cobblestone_albedo.dds");

    textureSpecification.DebugName = "Cobblestone Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\cobblestone\\Cobblestone_normal.dds");


    textureSpecification.DebugName = "Metal Steel Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\metal_steel\\MetalSteelBrushed_BaseColor.dds");

    textureSpecification.DebugName = "Metal Steel Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\metal_steel\\MetalSteelBrushed_Normal.dds");

    textureSpecification.DebugName = "Metal Steel Metalness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\metal_steel\\MetalSteelBrushed_Metallic.dds");

    textureSpecification.DebugName = "Metal Steel Roughness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\metal_steel\\MetalSteelBrushed_Roughness.dds");


    textureSpecification.DebugName = "Mudroad Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\mudroad\\MudRoad_albedo.dds");

    textureSpecification.DebugName = "Mudroad Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\mudroad\\MudRoad_normal.dds");


    textureSpecification.DebugName = "Crystall Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\crystall\\Crystal_COLOR.dds");

    textureSpecification.DebugName = "Crystall Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\cube\\crystall\\Crystal_NORM.dds");

    // Flashlight textures
    textureSpecification.DebugName = "Flashlight Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\flashlight\\Flashlight_Base_color.dds");

    textureSpecification.DebugName = "Flashlight Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\flashlight\\Flashlight_Normal.dds");

    textureSpecification.DebugName = "Flashlight Metalness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\flashlight\\Flashlight_Metallic.dds");

    textureSpecification.DebugName = "Flashlight Roughness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\flashlight\\Flashlight_Roughness.dds");

    // Samurai textures
    textureSpecification.DebugName = "Sword Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Sword_BaseColor.dds");

    textureSpecification.DebugName = "Sword Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Sword_Normal.dds");

    textureSpecification.DebugName = "Sword Metalness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Sword_Metallic.dds");

    textureSpecification.DebugName = "Sword Roughness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Sword_Roughness.dds");

    textureSpecification.DebugName = "Head Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Head_BaseColor.dds");

    textureSpecification.DebugName = "Head Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Head_Normal.dds");

    textureSpecification.DebugName = "Head Roughness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Head_Roughness.dds");

    textureSpecification.DebugName = "Eyes Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Eyes_BaseColor.dds");

    textureSpecification.DebugName = "Eyes Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Eyes_Normal.dds");

    textureSpecification.DebugName = "Helmet Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Helmet_BaseColor.dds");

    textureSpecification.DebugName = "Helmet Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Helmet_Normal.dds");

    textureSpecification.DebugName = "Helmet Metalness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Helmet_Metallic.dds");

    textureSpecification.DebugName = "Helmet Roughness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Helmet_Roughness.dds");

    textureSpecification.DebugName = "Decor Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Decor_BaseColor.dds");

    textureSpecification.DebugName = "Decor Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Decor_Normal.dds");

    textureSpecification.DebugName = "Decor Metalness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Decor_Metallic.dds");

    textureSpecification.DebugName = "Decor Roughness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Decor_Roughness.dds");

    textureSpecification.DebugName = "Pants Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Pants_BaseColor.dds");

    textureSpecification.DebugName = "Pants Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Pants_Normal.dds");

    textureSpecification.DebugName = "Pants Metalness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Pants_Metallic.dds");

    textureSpecification.DebugName = "Pants Roughness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Pants_Roughness.dds");

    textureSpecification.DebugName = "Hands Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Hands_BaseColor.dds");

    textureSpecification.DebugName = "Hands Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Hands_Normal.dds");

    textureSpecification.DebugName = "Hands Roughness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Hands_Roughness.dds");

    textureSpecification.DebugName = "Torso Albedo";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Torso_BaseColor.dds");

    textureSpecification.DebugName = "Torso Normal";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Torso_Normal.dds");

    textureSpecification.DebugName = "Torso Metalness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Torso_Metallic.dds");

    textureSpecification.DebugName = "Torso Roughness";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "models\\samurai\\Torso_Roughness.dds");

    textureSpecification.DebugName = "Dissolution Noise Map";
    textureLibrary->LoadTexture2D(textureSpecification, textureDirectoryPath / "Noise_2.dds");
}

void WorldLayer::AddPBRSamuraiToScene(std::string_view shaderName, const std::initializer_list<std::pair<std::string, DLEngine::Buffer>>& instanceData)
{
    auto& sceneMeshRegistry{ m_Scene->GetMeshRegistry() };

    const auto& textureLibrary{ DLEngine::Renderer::GetTextureLibrary() };
    const auto& textureDirectoryPath{ DLEngine::Texture::GetTextureDirectoryPath() };

    const auto& samurai{ DLEngine::Renderer::GetMeshLibrary()->Get("samurai") };
    const auto& shader{ DLEngine::Renderer::GetShaderLibrary()->Get(shaderName) };

    const auto baseTransform{ DLEngine::Math::Mat4x4::Rotate(DLEngine::Math::ToRadians(-90.0f), 0.0f, 0.0f) };

    auto samuraiInstance{ DLEngine::Instance::Create(shader, std::format("{0} Samurai Instance", shaderName).c_str()) };
    for (const auto& [name, buffer] : instanceData)
    {
        auto setBuffer{ buffer };

        if (name == "TRANSFORM")
        {
            const DLEngine::Math::Mat4x4 transform{ baseTransform * buffer.Read<DLEngine::Math::Mat4x4>() };
            setBuffer = DLEngine::Buffer::Copy(&transform, sizeof(DLEngine::Math::Mat4x4));
        }

        samuraiInstance->Set(name, setBuffer);
    }

    auto baseSamuraiMaterial{ DLEngine::Material::Create(shader, std::format("{0} Base Samurai Material", shaderName).c_str()) };
    if (shaderName == "GBuffer_PBR_Static_Dissolution")
    {
        const auto& dissolutionMap{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "Noise_2.dds")) };
        baseSamuraiMaterial->Set("t_DissolutionNoiseMap", dissolutionMap);
    }

    const auto& swordAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Sword_BaseColor.dds")) };
    const auto& swordNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Sword_Normal.dds")) };
    const auto& swordMetalness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Sword_Metallic.dds")) };
    const auto& swordRoughness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Sword_Roughness.dds")) };

    auto swordPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
    DLEngine::CBPBRMaterial swordCBPBRMaterial{};
    swordCBPBRMaterial.UseNormalMap = true;
    swordCBPBRMaterial.FlipNormalMapY = false;
    swordCBPBRMaterial.HasMetalnessMap = true;
    swordCBPBRMaterial.HasRoughnessMap = true;
    swordPBRCB->SetData(DLEngine::Buffer{ &swordCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

    auto swordMaterial{ DLEngine::Material::Copy(baseSamuraiMaterial, std::format("{0} Samurai Sword Material", shaderName).c_str()) };
    swordMaterial->Set("t_Albedo", swordAlbedo);
    swordMaterial->Set("t_Normal", swordNormal);
    swordMaterial->Set("t_Metalness", swordMetalness);
    swordMaterial->Set("t_Roughness", swordRoughness);
    swordMaterial->Set("PBRMaterial", swordPBRCB);

    sceneMeshRegistry.AddSubmesh(samurai, 0u, swordMaterial, samuraiInstance);

    const auto& headAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Head_BaseColor.dds")) };
    const auto& headNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Head_Normal.dds")) };
    const auto& headRoughness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Head_Roughness.dds")) };

    auto headPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
    DLEngine::CBPBRMaterial headCBPBRMaterial{};
    headCBPBRMaterial.UseNormalMap = true;
    headCBPBRMaterial.FlipNormalMapY = false;
    headCBPBRMaterial.HasMetalnessMap = false;
    headCBPBRMaterial.DefaultMetalness = 0.0f;
    headCBPBRMaterial.HasRoughnessMap = true;
    headPBRCB->SetData(DLEngine::Buffer{ &headCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

    auto headMaterial{ DLEngine::Material::Copy(baseSamuraiMaterial, std::format("{0} Samurai Head Material", shaderName).c_str()) };
    headMaterial->Set("t_Albedo", headAlbedo);
    headMaterial->Set("t_Normal", headNormal);
    headMaterial->Set("t_Roughness", headRoughness);
    headMaterial->Set("PBRMaterial", headPBRCB);

    sceneMeshRegistry.AddSubmesh(samurai, 1u, headMaterial, samuraiInstance);

    const auto& eyesAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Eyes_BaseColor.dds")) };
    const auto& eyesNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Eyes_Normal.dds")) };

    auto eyesPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
    DLEngine::CBPBRMaterial eyesCBPBRMaterial{};
    eyesCBPBRMaterial.UseNormalMap = true;
    eyesCBPBRMaterial.FlipNormalMapY = false;
    eyesCBPBRMaterial.HasMetalnessMap = false;
    eyesCBPBRMaterial.DefaultMetalness = 0.0f;
    eyesCBPBRMaterial.HasRoughnessMap = false;
    eyesCBPBRMaterial.DefaultRoughness = 0.0f;
    eyesPBRCB->SetData(DLEngine::Buffer{ &eyesCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

    auto eyesMaterial{ DLEngine::Material::Copy(baseSamuraiMaterial, std::format("{0} Samurai Eyes Material", shaderName).c_str()) };
    eyesMaterial->Set("t_Albedo", eyesAlbedo);
    eyesMaterial->Set("t_Normal", eyesNormal);
    eyesMaterial->Set("PBRMaterial", eyesPBRCB);

    sceneMeshRegistry.AddSubmesh(samurai, 2u, eyesMaterial, samuraiInstance);

    const auto& helmetAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Helmet_BaseColor.dds")) };
    const auto& helmetNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Helmet_Normal.dds")) };
    const auto& helmetMetalness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Helmet_Metallic.dds")) };
    const auto& helmetRoughness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Helmet_Roughness.dds")) };

    auto helmetPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
    DLEngine::CBPBRMaterial helmetCBPBRMaterial{};
    helmetCBPBRMaterial.UseNormalMap = true;
    helmetCBPBRMaterial.FlipNormalMapY = false;
    helmetCBPBRMaterial.HasMetalnessMap = true;
    helmetCBPBRMaterial.HasRoughnessMap = true;
    helmetPBRCB->SetData(DLEngine::Buffer{ &helmetCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

    auto helmetMaterial{ DLEngine::Material::Copy(baseSamuraiMaterial, std::format("{0} Samurai Helmet Material", shaderName).c_str()) };
    helmetMaterial->Set("t_Albedo", helmetAlbedo);
    helmetMaterial->Set("t_Normal", helmetNormal);
    helmetMaterial->Set("t_Metalness", helmetMetalness);
    helmetMaterial->Set("t_Roughness", helmetRoughness);
    helmetMaterial->Set("PBRMaterial", helmetPBRCB);

    sceneMeshRegistry.AddSubmesh(samurai, 3u, helmetMaterial, samuraiInstance);

    const auto& decorAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Decor_BaseColor.dds")) };
    const auto& decorNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Decor_Normal.dds")) };
    const auto& decorMetalness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Decor_Metallic.dds")) };
    const auto& decorRoughness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Decor_Roughness.dds")) };

    auto decorPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
    DLEngine::CBPBRMaterial decorCBPBRMaterial{};
    decorCBPBRMaterial.UseNormalMap = true;
    decorCBPBRMaterial.FlipNormalMapY = false;
    decorCBPBRMaterial.HasMetalnessMap = true;
    decorCBPBRMaterial.HasRoughnessMap = true;
    decorPBRCB->SetData(DLEngine::Buffer{ &decorCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

    auto decorMaterial{ DLEngine::Material::Copy(baseSamuraiMaterial, std::format("{0} Samurai Decor Material", shaderName).c_str()) };
    decorMaterial->Set("t_Albedo", decorAlbedo);
    decorMaterial->Set("t_Normal", decorNormal);
    decorMaterial->Set("t_Metalness", decorMetalness);
    decorMaterial->Set("t_Roughness", decorRoughness);
    decorMaterial->Set("PBRMaterial", decorPBRCB);

    sceneMeshRegistry.AddSubmesh(samurai, 4u, decorMaterial, samuraiInstance);

    const auto& pantsAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Pants_BaseColor.dds")) };
    const auto& pantsNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Pants_Normal.dds")) };
    const auto& pantsMetalness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Pants_Metallic.dds")) };
    const auto& pantsRoughness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Pants_Roughness.dds")) };

    auto pantsPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
    DLEngine::CBPBRMaterial pantsCBPBRMaterial{};
    pantsCBPBRMaterial.UseNormalMap = true;
    pantsCBPBRMaterial.FlipNormalMapY = false;
    pantsCBPBRMaterial.HasMetalnessMap = true;
    pantsCBPBRMaterial.HasRoughnessMap = true;
    pantsPBRCB->SetData(DLEngine::Buffer{ &pantsCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

    auto pantsMaterial{ DLEngine::Material::Copy(baseSamuraiMaterial, std::format("{0} Samurai Pants Material", shaderName).c_str()) };
    pantsMaterial->Set("t_Albedo", pantsAlbedo);
    pantsMaterial->Set("t_Normal", pantsNormal);
    pantsMaterial->Set("t_Metalness", pantsMetalness);
    pantsMaterial->Set("t_Roughness", pantsRoughness);
    pantsMaterial->Set("PBRMaterial", pantsPBRCB);

    sceneMeshRegistry.AddSubmesh(samurai, 5u, pantsMaterial, samuraiInstance);

    const auto& handsAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Hands_BaseColor.dds")) };
    const auto& handsNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Hands_Normal.dds")) };
    const auto& handsRoughness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Hands_Roughness.dds")) };

    auto handsPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
    DLEngine::CBPBRMaterial handsCBPBRMaterial{};
    handsCBPBRMaterial.UseNormalMap = true;
    handsCBPBRMaterial.FlipNormalMapY = false;
    handsCBPBRMaterial.HasMetalnessMap = false;
    handsCBPBRMaterial.DefaultMetalness = 0.0f;
    handsCBPBRMaterial.HasRoughnessMap = true;
    handsPBRCB->SetData(DLEngine::Buffer{ &handsCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

    auto handsMaterial{ DLEngine::Material::Copy(baseSamuraiMaterial, std::format("{0} Samurai Hands Material", shaderName).c_str()) };
    handsMaterial->Set("t_Albedo", handsAlbedo);
    handsMaterial->Set("t_Normal", handsNormal);
    handsMaterial->Set("t_Roughness", handsRoughness);
    handsMaterial->Set("PBRMaterial", handsPBRCB);

    sceneMeshRegistry.AddSubmesh(samurai, 6u, handsMaterial, samuraiInstance);

    const auto& torsoAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Torso_BaseColor.dds")) };
    const auto& torsoNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Torso_Normal.dds")) };
    const auto& torsoMetalness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Torso_Metallic.dds")) };
    const auto& torsoRoughness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\samurai\\Torso_Roughness.dds")) };

    auto torsoPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
    DLEngine::CBPBRMaterial torsoCBPBRMaterial{};
    torsoCBPBRMaterial.UseNormalMap = true;
    torsoCBPBRMaterial.FlipNormalMapY = false;
    torsoCBPBRMaterial.HasMetalnessMap = true;
    torsoCBPBRMaterial.HasRoughnessMap = true;
    torsoPBRCB->SetData(DLEngine::Buffer{ &torsoCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

    auto torsoMaterial{ DLEngine::Material::Copy(baseSamuraiMaterial, std::format("{0} Samurai Torso Material", shaderName).c_str()) };
    torsoMaterial->Set("t_Albedo", torsoAlbedo);
    torsoMaterial->Set("t_Normal", torsoNormal);
    torsoMaterial->Set("t_Metalness", torsoMetalness);
    torsoMaterial->Set("t_Roughness", torsoRoughness);
    torsoMaterial->Set("PBRMaterial", torsoPBRCB);

    const DLEngine::MeshRegistry::MeshUUID samuraiUUID{ sceneMeshRegistry.AddSubmesh(samurai, 7u, torsoMaterial, samuraiInstance) };

    if (shaderName == "GBuffer_PBR_Static_Dissolution")
        m_DissolutionGroupMeshes.emplace(samuraiUUID);
}

void WorldLayer::AddObjectsToScene()
{
    auto& sceneMeshRegistry{ m_Scene->GetMeshRegistry() };
    
    const auto& meshLibrary{ DLEngine::Renderer::GetMeshLibrary() };
    
    const auto& textureLibrary{ DLEngine::Renderer::GetTextureLibrary() };
    const auto& textureDirectoryPath{ DLEngine::Texture::GetTextureDirectoryPath() };
    
    const auto& pbrStaticShader{ DLEngine::Renderer::GetShaderLibrary()->Get("GBuffer_PBR_Static") };

    const auto& cube{ meshLibrary->Get("cube") };
    const auto& flashlight{ meshLibrary->Get("flashlight") };
    
    // Spawning cubes
    {
        // Cobblestone cube material
        auto pbrCobblestoneMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Cube Cobblestone Material") };
    
        const auto& cobblestoneAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\cobblestone\\Cobblestone_albedo.dds")) };
        const auto& cobblestoneNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\cobblestone\\Cobblestone_normal.dds")) };

        auto cobblestionePBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial steelCBPBRMaterial{};
        steelCBPBRMaterial.UseNormalMap = true;
        steelCBPBRMaterial.FlipNormalMapY = false;
        steelCBPBRMaterial.HasMetalnessMap = false;
        steelCBPBRMaterial.DefaultMetalness = 0.0f;
        steelCBPBRMaterial.HasRoughnessMap = false;
        steelCBPBRMaterial.DefaultRoughness = 1.0f;
        cobblestionePBRCB->SetData(DLEngine::Buffer{ &steelCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        pbrCobblestoneMaterial->Set("t_Albedo", cobblestoneAlbedo);
        pbrCobblestoneMaterial->Set("t_Normal", cobblestoneNormal);
        pbrCobblestoneMaterial->Set("PBRMaterial", cobblestionePBRCB);

        // Metal steel cube material
        auto pbrMetalSteelMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Cube Metal Steel Material") };

        const auto& metalSteelAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\metal_steel\\MetalSteelBrushed_BaseColor.dds")) };
        const auto& metalSteelNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\metal_steel\\MetalSteelBrushed_Normal.dds")) };
        const auto& metalSteelMetalness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\metal_steel\\MetalSteelBrushed_Metallic.dds")) };
        const auto& metalSteelRoughness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\metal_steel\\MetalSteelBrushed_Roughness.dds")) };

        auto metalSteelPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial metalSteelCBPBRMaterial{};
        metalSteelCBPBRMaterial.UseNormalMap = true;
        metalSteelCBPBRMaterial.FlipNormalMapY = false;
        metalSteelCBPBRMaterial.HasMetalnessMap = true;
        metalSteelCBPBRMaterial.HasRoughnessMap = true;
        metalSteelPBRCB->SetData(DLEngine::Buffer{ &metalSteelCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        pbrMetalSteelMaterial->Set("t_Albedo", metalSteelAlbedo);
        pbrMetalSteelMaterial->Set("t_Normal", metalSteelNormal);
        pbrMetalSteelMaterial->Set("t_Metalness", metalSteelMetalness);
        pbrMetalSteelMaterial->Set("t_Roughness", metalSteelRoughness);
        pbrMetalSteelMaterial->Set("PBRMaterial", metalSteelPBRCB);

        // Mudroad cube material
        auto pbrMudroadMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Cube Mudroad Material") };

        const auto& mudroadAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\mudroad\\MudRoad_albedo.dds")) };
        const auto& mudroadNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\mudroad\\MudRoad_normal.dds")) };

        auto mudroadPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial mudroadCBPBRMaterial{};
        mudroadCBPBRMaterial.UseNormalMap = true;
        mudroadCBPBRMaterial.FlipNormalMapY = false;
        mudroadCBPBRMaterial.HasMetalnessMap = false;
        mudroadCBPBRMaterial.DefaultMetalness = 0.0f;
        mudroadCBPBRMaterial.HasRoughnessMap = false;
        mudroadCBPBRMaterial.DefaultRoughness = 1.0f;
        mudroadPBRCB->SetData(DLEngine::Buffer{ &mudroadCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        pbrMudroadMaterial->Set("t_Albedo", mudroadAlbedo);
        pbrMudroadMaterial->Set("t_Normal", mudroadNormal);
        pbrMudroadMaterial->Set("PBRMaterial", mudroadPBRCB);

        // Crystall cube material
        auto pbrCrystallMaterial{ DLEngine::Material::Create(pbrStaticShader, "PBR_Static Cube Crystall Material") };

        const auto& crystallAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\crystall\\Crystal_COLOR.dds")) };
        const auto& crystallNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\cube\\crystall\\Crystal_NORM.dds")) };

        auto crystallPBRCB{ DLEngine::ConstantBuffer::Create(sizeof(DLEngine::CBPBRMaterial)) };
        DLEngine::CBPBRMaterial crystallCBPBRMaterial{};
        crystallCBPBRMaterial.UseNormalMap = true;
        crystallCBPBRMaterial.FlipNormalMapY = true;
        crystallCBPBRMaterial.HasMetalnessMap = false;
        crystallCBPBRMaterial.DefaultMetalness = 0.0f;
        crystallCBPBRMaterial.HasRoughnessMap = false;
        crystallCBPBRMaterial.DefaultRoughness = 0.3f;
        crystallPBRCB->SetData(DLEngine::Buffer{ &crystallCBPBRMaterial, sizeof(DLEngine::CBPBRMaterial) });

        pbrCrystallMaterial->Set("t_Albedo", crystallAlbedo);
        pbrCrystallMaterial->Set("t_Normal", crystallNormal);
        pbrCrystallMaterial->Set("PBRMaterial", crystallPBRCB);

#if 0
        const auto transform{ DLEngine::Math::Mat4x4::Scale(DLEngine::Math::Vec3{ 10.0f, 0.5f, 10.0f }) *
            DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 0.0f, -0.5f, 0.0f })
        };
        auto instance{ DLEngine::Instance::Create(pbrStaticShader, "PBR_Static Cube Instance") };
        instance->Set("TRANSFORM", DLEngine::Buffer{ &transform, sizeof(DLEngine::Math::Mat4x4) });

        sceneMeshRegistry.AddSubmesh(cube, 0u, pbrCobblestoneMaterial, instance);
#else

        for (int32_t x{ -5 }; x < 5; ++x)
        {
            for (int32_t z{ -5 }; z < 5; ++z)
            {
                const DLEngine::Math::Mat4x4 transform{ DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ static_cast<float>(x), -1.0f, static_cast<float>(z) }) };
                const auto& instance{ DLEngine::Instance::Create(pbrStaticShader, "PBR_Static Cube Instance") };
                instance->Set("TRANSFORM", DLEngine::Buffer{ &transform, sizeof(DLEngine::Math::Mat4x4) });

                DLEngine::Ref<DLEngine::Material> pbrMaterial;
                if (x < 0 && z < 0)
                    pbrMaterial = pbrCobblestoneMaterial;
                else if (x < 0 && z >= 0)
                    pbrMaterial = pbrMudroadMaterial;
                else if (x >= 0 && z < 0)
                    pbrMaterial = pbrCrystallMaterial;
                else
                    pbrMaterial = pbrMetalSteelMaterial;

                sceneMeshRegistry.AddSubmesh(cube, 0u, pbrMaterial, instance);
            }
        }
#endif

    }

    // Adding samurais
    const auto& transforms{ std::vector<DLEngine::Math::Mat4x4>{
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -1.5f, 0.0f, 2.0f }),
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{  1.5f, 0.0f, 2.0f }),
    } };

    AddPBRSamuraiToScene("GBuffer_PBR_Static",
        {
            { "TRANSFORM", DLEngine::Buffer{ &transforms[0u], sizeof(DLEngine::Math::Mat4x4) } },
        });

    AddPBRSamuraiToScene("GBuffer_PBR_Static",
        {
            { "TRANSFORM", DLEngine::Buffer{ &transforms[1u], sizeof(DLEngine::Math::Mat4x4) } },
        });

    // Adding lights
    {
        m_Scene->AddDirectionalLight(DLEngine::Math::Normalize(DLEngine::Math::Vec3{ -1.0f, -1.0f, 1.0f }), DLEngine::Math::Vec3{ 3.66e+3f, 3.66e+3f, 4.08e+3f }, 6.8e-5f);
        m_Scene->AddPointLight(DLEngine::Math::Vec3{ 0.0f }, DLEngine::Math::Vec3{ 1.0f }, 0.1f, 1.0f, DLEngine::Math::Vec3{ -1.5f, 3.0f, 1.0f });
        m_Scene->AddPointLight(DLEngine::Math::Vec3{ 0.0f }, DLEngine::Math::Vec3{ 1.0f }, 0.3f, 1.0f, DLEngine::Math::Vec3{ 1.5f, 3.0f, 1.0f });
        m_Scene->AddPointLight(DLEngine::Math::Vec3{ 0.0f }, DLEngine::Math::Vec3{ 2.0f, 15.0f, 4.0f }, 0.3f, 0.7f, DLEngine::Math::Vec3{ 0.0f, 4.0f, 2.0f });

        m_FlashlightBaseTransform = DLEngine::Math::Mat4x4::Scale(DLEngine::Math::Vec3{ 0.003f }) *
            DLEngine::Math::Mat4x4::Rotate(DLEngine::Math::ToRadians(-90.0f), DLEngine::Math::ToRadians(180.0f), 0.0f);
        
        const auto& flashlightAlbedo{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\flashlight\\Flashlight_Base_color.dds")) };
        const auto& flashlightNormal{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\flashlight\\Flashlight_Normal.dds")) };
        const auto& flashlightMetalness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\flashlight\\Flashlight_Metallic.dds")) };
        const auto& flashlightRoughness{ AsRef<DLEngine::Texture2D>(textureLibrary->Get(textureDirectoryPath / "models\\flashlight\\Flashlight_Roughness.dds")) };

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

        DLEngine::Ref<DLEngine::Instance> flashlightInstance{ DLEngine::Instance::Create(pbrStaticShader, "PBR_Static Flashlight Instance") };
        flashlightInstance->Set("TRANSFORM", DLEngine::Buffer{ &m_FlashlightBaseTransform, sizeof(DLEngine::Math::Mat4x4) });

        m_FlashlightMeshUUID = sceneMeshRegistry.AddSubmesh(flashlight, 0u, flashlightMaterial, flashlightInstance);

        m_Scene->AddSpotLight(
            DLEngine::Math::Vec3{ 0.0f },
            DLEngine::Math::Vec3{ 0.0f, -1.0f, 0.0f },
            0.075f,
            DLEngine::Math::Cos(DLEngine::Math::ToRadians(5.0f)),
            DLEngine::Math::Cos(DLEngine::Math::ToRadians(7.5f)),
            DLEngine::Math::Vec3{ 4.0f, 1.0f, 15.0f },
            2.0f,
            m_FlashlightMeshUUID
        );
    }
}

void WorldLayer::SwapDissolutionGroupInstances(DLEngine::DeltaTime dt)
{
    using namespace DLEngine;

    auto& sceneMeshRegistry{ m_Scene->GetMeshRegistry() };

    std::erase_if(m_DissolutionGroupMeshes, [&](MeshRegistry::MeshUUID submeshUUID)
        {
            Ref<Instance> instance{ sceneMeshRegistry.GetInstance(submeshUUID) };

            const float dissolutionDuration{ instance->Get<float>("DISSOLUTION_DURATION") };
            const float elapsedTime{ instance->Get<float>("ELAPSED_TIME") };

            const float updatedElapsedTime{ elapsedTime + dt };
            instance->Set("ELAPSED_TIME", Buffer{ &updatedElapsedTime, sizeof(float) });

            if (elapsedTime < dissolutionDuration)
                return false;

            const Math::Mat4x4& transform{
                DLEngine::Math::Mat4x4::Rotate(DLEngine::Math::ToRadians(90.0f), 0.0f, 0.0f) *
                instance->Get<Math::Mat4x4>("TRANSFORM")
            };

            AddPBRSamuraiToScene("GBuffer_PBR_Static",
                {
                    { "TRANSFORM", DLEngine::Buffer{ &transform, sizeof(DLEngine::Math::Mat4x4) } },
                });

            sceneMeshRegistry.RemoveSubmesh(submeshUUID);

            return true;
        }
    );
}

bool WorldLayer::OnKeyPressedEvent(DLEngine::KeyPressedEvent& e)
{
    switch (e.GetKeyCode())
    {
    case 'F':
        m_IsFlashlightAttached = !m_IsFlashlightAttached;
        break;
    case 'M':
        {
            const auto& sceneCamera{ m_Scene->GetCamera() };
            const auto& sceneCameraPosition{ sceneCamera.GetPosition() };
            const auto& sceneCameraForward{ sceneCamera.GetForward() };
            const auto& samuraiPosition{ sceneCameraPosition + sceneCameraForward * m_DissolutionGroupSpawnSettings.DistanceToCamera };

            const auto& transform{ DLEngine::Math::Mat4x4::Inverse(DLEngine::Math::Mat4x4::LookTo(samuraiPosition, sceneCameraForward, sceneCamera.GetUp())) };
            const float duration{ DLEngine::RandomGenerator::GenerateRandom(m_DissolutionGroupSpawnSettings.MinDissolutionDuration, m_DissolutionGroupSpawnSettings.MaxDissolutionDuration) * 1.0e3f }; // Seconds to milliseconds
            const float elapsedTime{ 0.0f };

            AddPBRSamuraiToScene("GBuffer_PBR_Static_Dissolution",
                {
                    { "TRANSFORM"           , DLEngine::Buffer{ &transform  , sizeof(DLEngine::Math::Mat4x4) } },
                    { "DISSOLUTION_DURATION", DLEngine::Buffer{ &duration   , sizeof(float)                  } },
                    { "ELAPSED_TIME"        , DLEngine::Buffer{ &elapsedTime, sizeof(float)                  } },
                });
        } break;
    case 'G':
    {
        const auto& camera{ m_Scene->GetCamera() };
        const auto& cursorPos{ DLEngine::Input::GetCursorPosition() };
        DLEngine::Math::Vec3 cursorPosNDC{
            cursorPos.x / static_cast<float>(m_Scene->GetViewportWidth()) * 2.0f - 1.0f,
            (1.0f - cursorPos.y / static_cast<float>(m_Scene->GetViewportHeight())) * 2.0f - 1.0f,
            1.0f
        };

        DLEngine::Math::Ray ray{};
        ray.Origin = camera.GetPosition();
        ray.Direction = DLEngine::Math::Normalize(camera.ConstructFrustumPosNoTranslation(cursorPosNDC));

        const DLEngine::Math::Vec3 decalTintColor{
            DLEngine::RandomGenerator::GenerateRandom(0.0f, 1.0f),
            DLEngine::RandomGenerator::GenerateRandom(0.0f, 1.0f),
            DLEngine::RandomGenerator::GenerateRandom(0.0f, 1.0f)
        };

        const float decalRotation{
            DLEngine::RandomGenerator::GenerateRandom(-2.0f * DLEngine::Math::Numeric::Pi, 2.0f * DLEngine::Math::Numeric::Pi)
        };

        m_Scene->SpawnDecal(ray, decalTintColor, decalRotation);
    } break;
    default:
        break;
    }

    return false;
}
