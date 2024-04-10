#pragma once
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Entity/Entity.h"

class WorldLayer
    : public Layer
{
public:
    ~WorldLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float dt) override;

private:
    void ProcessInputs();

private:
    Math::Sphere m_Sphere { { 0.0f, 0.0f, 5.0f }, 1.0f };

    float m_EntitySpeed { 0.01f };
    Math::Vec2f m_MoveDirection { 0.0f, 0.0f };

    Math::Vec2<int32_t> m_PrevMousePosition { 0, 0 };
    bool m_MouseStartedDragging { false };

    bool m_ShouldDraw { true };
};

