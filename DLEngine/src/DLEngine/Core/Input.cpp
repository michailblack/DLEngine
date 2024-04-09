#include "dlpch.h"
#include "Input.h"

#include "DLEngine/Renderer/Renderer.h"

bool Input::IsKeyPressed(uint8_t keyCode)
{
    return Renderer::GetWindow()->Keyboard.IsKeyPressed(keyCode);
}

bool Input::IsMouseButtonPressed(Mouse::Button button)
{
    return Renderer::GetWindow()->Mouse.IsButtonPressed(button);
}

Math::Vec2<int32_t> Input::GetMousePosition()
{
    return Renderer::GetWindow()->Mouse.GetPosition();
}

int32_t Input::GetMouseX()
{
    return GetMousePosition().Data[0];
}

int32_t Input::GetMouseY()
{
    return GetMousePosition().Data[1];
}
