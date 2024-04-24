#pragma once
#include "DLEngine/Math/Vec2.h"

class Input
{
    friend class Window;
public:
    static bool IsKeyPressed(uint8_t keyCode) noexcept;
    static Math::Vec2 GetCursorPosition() noexcept;
    static int32_t GetMouseX() noexcept;
    static int32_t GetMouseY() noexcept;

private:
    static void OnKeyPressed(uint8_t keyCode) noexcept;
    static void OnKeyReleased(uint8_t keyCode) noexcept;

    static void OnMouseMove(int32_t x, int32_t y) noexcept;

    static void ResetKeys() noexcept;
};
