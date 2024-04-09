#pragma once
#include "DLEngine/Core/Window.h"
#include "DLEngine/Entity/Entity.h"

class Renderer
{
public:
    static void SetWindow(const Ref<Window>& window);
    static Ref<Window>& GetWindow();

    static void Draw(const Math::Sphere& entity);

    static Math::Vec2f ScreenSpaceToWorldSpace(const Math::Vec2<int32_t>& screenPos);
};
