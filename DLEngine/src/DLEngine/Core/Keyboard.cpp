#include "dlpch.h"
#include "Keyboard.h"

bool Keyboard::IsKeyPressed(uint8_t keyCode) const
{
    return m_KeysStates[keyCode];
}

void Keyboard::OnKeyPressed(uint8_t keyCode)
{
    m_KeysStates[keyCode] = true;
}

void Keyboard::OnKeyReleased(uint8_t keyCode)
{
    m_KeysStates[keyCode] = false;
}

void Keyboard::ResetKeys()
{
    m_KeysStates.reset();
}
