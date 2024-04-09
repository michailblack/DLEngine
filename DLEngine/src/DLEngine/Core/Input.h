#pragma once
#include "Mouse.h"

class Input
{
public:
    static bool IsKeyPressed(uint8_t keyCode);
    static bool IsMouseButtonPressed(Mouse::Button button);

    static Math::Vec2<int32_t> GetMousePosition();
    static int32_t GetMouseX();
    static int32_t GetMouseY();
};
