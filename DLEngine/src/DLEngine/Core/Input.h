#pragma once
#include <bitset>

#include "DLEngine/Math/Vec.h"

class Input
{
    friend class Window;
public:
    ~Input() = default;

    Input(const Input& other) = delete;
    Input(Input&& other) = delete;
    Input& operator=(const Input& other) = delete;
    Input& operator=(Input&& other) = delete;

    static Input& Get()
    {
        static Input s_Instance;
        return s_Instance;
    }

    bool IsKeyPressed(uint8_t keyCode) const;
    Math::Vec2<int32_t> GetCursorPosition() const;
    int32_t GetMouseX() const;
    int32_t GetMouseY() const;

private:
    Input() = default;

    void OnKeyPressed(uint8_t keyCode);
    void OnKeyReleased(uint8_t keyCode);

    void OnMouseMove(int32_t x, int32_t y);

    void ResetKeys();

private:
    inline static constexpr uint8_t s_KeyCount { 255u };

    std::bitset<s_KeyCount> m_KeysStates;

    int32_t m_MouseX { 0 };
    int32_t m_MouseY { 0 };
};
