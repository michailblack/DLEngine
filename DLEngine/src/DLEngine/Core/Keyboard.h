#pragma once
#include <bitset>

class Keyboard
{
    friend class Window;
public:
    Keyboard() = default;
    ~Keyboard() = default;

    Keyboard(const Keyboard&) = delete;
    Keyboard(Keyboard&&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;
    Keyboard& operator=(Keyboard&&) = delete;

    bool IsKeyPressed(uint8_t keyCode) const;

private:
    void OnKeyPressed(uint8_t keyCode);
    void OnKeyReleased(uint8_t keyCode);
    void ResetKeys();

private:
    inline static constexpr uint8_t s_KeyCount { 255u };
    std::bitset<s_KeyCount> m_KeysStates;
};
