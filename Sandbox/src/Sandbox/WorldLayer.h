#pragma once
#include "DLEngine/Core/Layer.h"
#include "DLEngine/Core/Events/ApplicationEvent.h"

#include "DLEngine/Entity/Entity.h"

#include "DLEngine/Renderer/CameraController.h"

class WorldLayer
    : public Layer
{
public:
    WorldLayer();
    ~WorldLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float dt) override;
    void OnEvent(Event& e) override;

private:
    CameraController m_CameraController;

    Math::Sphere m_Sphere { Math::Vec3{ 0.0f, 0.0f, 5.0f }, 1.0f };
};

