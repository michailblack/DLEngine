#pragma once
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Renderer/Scene.h"
#include "DLEngine/Renderer/SceneRenderer.h"

struct DissolutionGroupSpawnSettings
{
    float DistanceToCamera{ 2.5f };
    float MinDissolutionDuration{ 3.0f };
    float MaxDissolutionDuration{ 7.0f };
};

class WorldLayer : public DLEngine::Layer
{
public:
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(DLEngine::DeltaTime dt) override;
    void OnImGuiRender() override;
    void OnEvent(DLEngine::Event& e) override;

private:
    void LoadMeshes();
    void LoadTextures();
    void AddPBRSamuraiToScene(std::string_view shaderName, const std::initializer_list<std::pair<std::string, DLEngine::Buffer>>& instanceData);
    void AddObjectsToScene();

private:
    bool OnKeyPressedEvent(DLEngine::KeyPressedEvent& e);

private:
    DLEngine::Ref<DLEngine::Scene> m_Scene{};
    DLEngine::Ref<DLEngine::SceneRenderer> m_SceneRenderer{};

    DLEngine::PBRSettings m_PBRSettings{};
    DLEngine::PostProcessingSettings m_PostProcessingSettings{};
    DLEngine::ShadowMappingSettings m_ShadowMappingSettings{};

    DLEngine::Math::Mat4x4 m_FlashlightBaseTransform{};
    DLEngine::Ref<DLEngine::Instance> m_FlashlightInstance{};
    bool m_IsFlashlightAttached{ true };

    DissolutionGroupSpawnSettings m_DissolutionGroupSpawnSettings{};

    float m_Time{ 0.0f };
    float m_DeltaTime{ 0.0f };
};
