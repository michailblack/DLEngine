#pragma once
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Renderer/CameraController.h"

class WorldLayer
    : public DLEngine::Layer
{
public:
    WorldLayer();
    ~WorldLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(DeltaTime dt) override;
    void OnEvent(DLEngine::Event& e) override;

private:
    void InitHologramGroup() const;
    void InitTextureOnlyGroup() const;
    void InitEmissionGroup() const;
    void InitLitGroup() const;

    bool OnKeyPressedEvent(DLEngine::KeyPressedEvent& e);

    bool OnKeyPressed(DLEngine::KeyPressedEvent& e);

private:
    DLEngine::CameraController m_CameraController;
    uint32_t m_CameraTransformID{ 0u };
    bool m_IsFlashlightAttached{ true };
    float m_EV100{ -2.0f };
};
