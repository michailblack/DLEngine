#include "dlpch.h"
#include "Mouse.h"

bool Mouse::IsButtonPressed(Button button) const
{
    return m_Buttons[static_cast<size_t>(button)];
}

Math::Vec2<int32_t> Mouse::GetPosition() const
{
    return { m_X, m_Y };
}

int32_t Mouse::GetX() const
{
    return m_X;
}

int32_t Mouse::GetY() const
{
    return m_Y;
}

void Mouse::OnMouseMove(uint32_t x, uint32_t y)
{
    m_X = x;
    m_Y = y;
}

void Mouse::OnMouseButtonPressed(Button button, uint32_t x, uint32_t y)
{
    m_Buttons[static_cast<size_t>(button)] = true;
    m_X = x;
    m_Y = y;
}

void Mouse::OnMouseButtonReleased(Button button, uint32_t x, uint32_t y)
{
    m_Buttons[static_cast<size_t>(button)] = false;
    m_X = x;
    m_Y = y;
}
