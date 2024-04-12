#pragma once
#include "DLEngine/Math/Vec2.h"

class Input
{
    friend class Window;
public:
    static bool IsKeyPressed(uint8_t keyCode);
    static Math::Vec2 GetCursorPosition();
    static int32_t GetMouseX();
    static int32_t GetMouseY();

private:
    static void OnKeyPressed(uint8_t keyCode);
    static void OnKeyReleased(uint8_t keyCode);

    static void OnMouseMove(int32_t x, int32_t y);

    static void ResetKeys();
};
