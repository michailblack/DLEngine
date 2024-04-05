#pragma once
#include <bitset>

#include "DLEngine/Math/Vec.h"

class Mouse
{
    friend class Window;
public:
    enum class Button
    {
        Left,
        Right,
        Middle,

        COUNT
    };
public:
    Mouse() = default;
    ~Mouse() = default;

    Mouse (const Mouse&) = delete;
    Mouse (Mouse&&) = delete;
    Mouse& operator=(const Mouse&) = delete;
    Mouse& operator=(Mouse&&) = delete;

    bool IsButtonPressed(Button button) const;
    Math::Vec2<int32_t> GetPosition() const;
    int32_t GetX() const;
    int32_t GetY() const;

private:
    void OnMouseMove(uint32_t x, uint32_t y);
    void OnMouseButtonPressed(Button button, uint32_t x, uint32_t y);
    void OnMouseButtonReleased(Button button, uint32_t x, uint32_t y);

private:
    std::bitset<static_cast<size_t>(Button::COUNT)> m_Buttons;
    int32_t m_X { 0 };
    int32_t m_Y { 0 };
};
