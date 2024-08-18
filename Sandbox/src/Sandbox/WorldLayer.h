#pragma once
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Renderer/Scene.h"
#include "DLEngine/Renderer/SceneRenderer.h"

class WorldLayer : public DLEngine::Layer
{
public:
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(DeltaTime dt) override;
    void OnImGuiRender() override;
    void OnEvent(DLEngine::Event& e) override;

private:
    void AddObjectsToScene();

private:
    bool OnKeyPressedEvent(DLEngine::KeyPressedEvent& e);

private:
    DLEngine::Ref<DLEngine::Scene> m_Scene{};
    DLEngine::Ref<DLEngine::SceneRenderer> m_SceneRenderer{};

    DLEngine::PBRSettings m_PBRSettings{};
    DLEngine::PostProcessSettings m_PostProcessSettings{};

    DLEngine::Math::Mat4x4 m_FlashlightBaseTransform{};
    DLEngine::Ref<DLEngine::Instance> m_FlashlightInstance{};
    bool m_IsFlashlightAttached{ true };
};
