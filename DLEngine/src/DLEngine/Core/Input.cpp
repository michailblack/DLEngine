#include "dlpch.h"
#include "Input.h"

#include <bitset>

namespace 
{
    struct InputData
    {
        static constexpr uint8_t s_KeyCount { 255u };

        std::bitset<s_KeyCount> m_KeysStates;

        int32_t m_MouseX { 0 };
        int32_t m_MouseY { 0 };
    } s_Data;
}

bool Input::IsKeyPressed(uint8_t keyCode)
{
    return s_Data.m_KeysStates[keyCode];
}

Math::Vec2 Input::GetCursorPosition()
{
    return Math::Vec2 { static_cast<float>(s_Data.m_MouseX), static_cast<float>(s_Data.m_MouseY) };
}

int32_t Input::GetMouseX()
{
    return s_Data.m_MouseX;
}

int32_t Input::GetMouseY()
{
    return s_Data.m_MouseY;
}

void Input::OnKeyPressed(uint8_t keyCode)
{
    s_Data.m_KeysStates[keyCode] = true;
}

void Input::OnKeyReleased(uint8_t keyCode)
{
    s_Data.m_KeysStates[keyCode] = false;
}

void Input::OnMouseMove(int32_t x, int32_t y)
{
    s_Data.m_MouseX = x;
    s_Data.m_MouseY = y;
}

void Input::ResetKeys()
{
    s_Data.m_KeysStates.reset();
}
