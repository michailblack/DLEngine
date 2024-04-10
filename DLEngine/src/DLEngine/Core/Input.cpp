#include "dlpch.h"
#include "Input.h"


bool Input::IsKeyPressed(uint8_t keyCode) const
{
    return m_KeysStates[keyCode];
}

Math::Vec2<int32_t> Input::GetCursorPosition() const
{
    return { m_MouseX, m_MouseY };
}

int32_t Input::GetMouseX() const
{
    return m_MouseX;
}

int32_t Input::GetMouseY() const
{
    return m_MouseY;
}

void Input::OnKeyPressed(uint8_t keyCode)
{
    m_KeysStates[keyCode] = true;
}

void Input::OnKeyReleased(uint8_t keyCode)
{
    m_KeysStates[keyCode] = false;
}

void Input::OnMouseMove(int32_t x, int32_t y)
{
    m_MouseX = x;
    m_MouseY = y;
}

void Input::ResetKeys()
{
    m_KeysStates.reset();
}
