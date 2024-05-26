#pragma once
#include <cstdint>

#define EXCLUDE_COMMON_WINDOWS_HEADERS
#include "DLEngine/Core/DLWin.h"

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "DLEngine/Core/Events/Event.h"
#include "DLEngine/Math/Vec2.h"

namespace DLEngine
{
    class Window
    {
        using EventCallbackFn = std::function<void(Event&)>;

    private:
        class WindowClass
        {
        public:
            WindowClass(const WindowClass&) = delete;
            WindowClass(WindowClass&&) = delete;
            WindowClass& operator=(const WindowClass&) = delete;
            WindowClass& operator=(WindowClass&&) = delete;

            static const wchar_t* GetName() noexcept { return m_WindowClassName; }
            static HINSTANCE& GetInstance() noexcept
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
        Window(Window&&) = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;

        void OnResize(uint32_t width, uint32_t height);

        void SetEventCallback(const EventCallbackFn& eventCallback) noexcept { m_Data.EventCallback = eventCallback; }

        Math::Vec2 GetSize() const noexcept { return Math::Vec2{ static_cast<float>(m_Data.Width), static_cast<float>(m_Data.Height) }; }
        uint32_t GetWidth() const noexcept { return m_Data.Width; }
        uint32_t GetHeight() const noexcept { return m_Data.Height; }

        HWND GetHandle() const noexcept { return m_hWnd; }

    private:
        static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK HandleMsgInitiate(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, const EventCallbackFn& eventCallback);

    private:
        HWND m_hWnd;

        struct WindowData
        {
            uint32_t Width {}, Height {};
            const wchar_t* Title {};

            EventCallbackFn EventCallback;
        } m_Data;
    };
}
