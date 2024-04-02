#pragma once
#include <vector>

#include "DLEngine/Core/Events/KeyEvent.h"
#include "DLEngine/Core/Events/MouseEvent.h"
#include "DLEngine/Entity/Entity.h"

class Scene
{
public:
    void Init();

    void OnUpdate(float dt);

    const std::vector<Math::Sphere>& GetEntities() const { return m_Entities; }

private:
    void OnKeyPressedEvent(const KeyPressedEvent& event);
    void OnMouseMovedEvent(const MouseMovedEvent& event);
    void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
    void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);

private:
    std::vector<Math::Sphere> m_Entities;
    Math::Vec2f m_MoveDirection { 0.0f, 0.0f };
    Math::Vec2<int32_t> m_MousePosition { 0, 0 };
    bool m_CanMove { false };
};

