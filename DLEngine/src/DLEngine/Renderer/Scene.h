#pragma once
#include "DLEngine/Entity/Entity.h"

class Scene
{
public:
    void Init();

    void OnUpdate(float dt);

private:
    void ProcessInputs();

private:
    Math::Sphere m_Sphere { { 0.0f, 0.0f, 5.0f }, 1.0f };

    float m_EntitySpeed { 0.01f };
    Math::Vec2f m_MoveDirection { 0.0f, 0.0f };

    Math::Vec2<int32_t> m_PrevMousePosition { 0, 0 };
    bool m_MouseWantsToMove { false };
    bool m_MouseStartedDragging { false };

    bool m_ShouldDraw { true };
};

