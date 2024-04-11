#pragma once
#include "DLEngine/Core/Layer.h"
#include "DLEngine/Core/Events/ApplicationEvent.h"

#include "DLEngine/Entity/Entity.h"

#include "DLEngine/Math/Vec2.h"

class WorldLayer
    : public Layer
{
public:
    ~WorldLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float dt) override;
    void OnEvent(Event& event) override;

private:
    void ProcessInputs();

    bool OnWindowResize(WindowResizeEvent& event);

private:
    Math::Sphere m_Sphere { { 0.0f, 0.0f, 5.0f }, 1.0f };

    float m_EntitySpeed { 0.01f };
    Math::Vec2 m_MoveDirection { 0.0f, 0.0f };

    Math::Vec2 m_PrevMousePosition { 0, 0 };
    bool m_MouseStartedDragging { false };

    bool m_ShouldDraw { true };
};

