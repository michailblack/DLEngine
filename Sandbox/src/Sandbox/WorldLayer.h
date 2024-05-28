#pragma once
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Systems/Renderer/CameraController.h"

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
    void InitNormalVisGroup() const;
    void InitHologramGroup() const;

private:
    DLEngine::CameraController m_CameraController;
};
