#pragma once
#include <cstdint>

#define EXCLUDE_COMMON_WINDOWS_HEADERS
#include "DLEngine/Core/DLWin.h"

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "DLEngine/Core/Events/Event.h"
#include "DLEngine/Math/Vec2.h"

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
    Window(uint32_t width, uint32_t height, const wchar_t* title, const EventCallbackFn& callback);
    ~Window();

    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    void Present() const;

    Math::Vec2 GetSize() const noexcept { return Math::Vec2 { static_cast<float>(m_Data.m_Width), static_cast<float>(m_Data.m_Height) }; }
    uint32_t GetWidth() const noexcept { return m_Data.m_Width; }
    uint32_t GeHeight() const noexcept { return m_Data.m_Height; }

    HWND GetHandle() const noexcept { return m_hWnd; }

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> GetRenderTargetView() const noexcept { return m_Data.m_RenderTargetView; }

private:
    static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK HandleMsgPassToWndMember(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hWnd;

    struct WindowData
    {
        uint32_t m_Width, m_Height;
        const wchar_t* m_Title;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> m_RenderTargetView;

        EventCallbackFn m_EventCallback;
    } m_Data;
};
