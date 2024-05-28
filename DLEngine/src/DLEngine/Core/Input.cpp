#include "dlpch.h"
#include "Input.h"

#include <bitset>

namespace DLEngine
{
    namespace
    {
        struct
        {
            static constexpr uint8_t s_KeyCount{ 255u };

            std::bitset<s_KeyCount> m_KeysStates;

            int32_t m_MouseX{ 0 };
            int32_t m_MouseY{ 0 };
        } s_InputData;
    }

    bool Input::IsKeyPressed(uint8_t keyCode) noexcept
    {
        return s_InputData.m_KeysStates[keyCode];
    }

    Math::Vec2 Input::GetCursorPosition() noexcept
    {
        return Math::Vec2{ static_cast<float>(s_InputData.m_MouseX), static_cast<float>(s_InputData.m_MouseY) };
    }

    int32_t Input::GetMouseX() noexcept
    {
        return s_InputData.m_MouseX;
    }

    int32_t Input::GetMouseY() noexcept
    {
        return s_InputData.m_MouseY;
    }

    void Input::OnKeyPressed(uint8_t keyCode) noexcept
    {
        s_InputData.m_KeysStates[keyCode] = true;
    }

    void Input::OnKeyReleased(uint8_t keyCode) noexcept
    {
        s_InputData.m_KeysStates[keyCode] = false;
    }

    void Input::OnMouseMove(int32_t x, int32_t y) noexcept
    {
        s_InputData.m_MouseX = x;
        s_InputData.m_MouseY = y;
    }

    void Input::ResetKeys() noexcept
    {
        s_InputData.m_KeysStates.reset();
    }
}
