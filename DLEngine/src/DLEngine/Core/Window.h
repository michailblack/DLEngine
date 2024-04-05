#pragma once
#include <cstdint>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Keyboard.h"
#include "Mouse.h"
#include "DLEngine/Math/Vec.h"

class Window
{
    friend LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
    class WindowClass
    {
    public:
        WindowClass(const WindowClass&) = delete;
        WindowClass(WindowClass&&) = delete;
        WindowClass& operator=(const WindowClass&) = delete;
        WindowClass& operator=(WindowClass&&) = delete;

        static const wchar_t* GetName() { return m_WindowClassName; }
        static HINSTANCE& GetInstance()
        {
            static WindowClass m_WindowClass;
            return m_WindowClass.m_hInstance;
        }
    private:
        WindowClass();
        ~WindowClass();
    private:
        inline static constexpr const wchar_t* m_WindowClassName = L"DLEngine Window Class";

        HINSTANCE m_hInstance;
    };
public:
    Window(uint32_t width, uint32_t height, const wchar_t* title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Math::Vec2<uint32_t> GetSize() const { return { m_Width, m_Height }; }
    Math::Vec2<uint32_t> GetFramebufferSize() const { return m_FramebufferSize; }

    std::vector<COLORREF>& GetFramebuffer() { return m_BitmapFramebuffer; }

    HWND GetHandle() const { return m_hWnd; }

    bool ShouldClose() const { return m_WindowShouldClose; }

public:
    Keyboard Keyboard;
    Mouse Mouse;

private:
    void OnResize(uint32_t width, uint32_t height);

    static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK HandleMsgPassToWndMember(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    uint32_t m_Width, m_Height;
    const wchar_t* m_Title;

    bool m_WindowShouldClose { false };

    std::vector<COLORREF> m_BitmapFramebuffer;
    Math::Vec2<uint32_t> m_FramebufferSize;

    HWND m_hWnd;
};
