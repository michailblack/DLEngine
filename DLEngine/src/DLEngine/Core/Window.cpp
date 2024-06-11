#include "dlpch.h"
#include "Window.h"

#include "DLEngine/Core/Input.h"

#include "DLEngine/Core/Events/ApplicationEvent.h"
#include "DLEngine/Core/Events/KeyEvent.h"
#include "DLEngine/Core/Events/MouseEvent.h"

#pragma comment(lib, "dxgi.lib")

namespace DLEngine
{
    Window::WindowClass::WindowClass()
    {
        if (GetModuleHandleExW(0, nullptr, &m_hInstance) == 0)
        {
            DL_THROW_LAST_WIN32();
        }

        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = HandleMsgSetup;
        wc.hInstance = m_hInstance;
        wc.hCursor = static_cast<HCURSOR>(LoadImageW(nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        wc.lpszClassName = m_WindowClassName;

        RegisterClassExW(&wc);
    }

    Window::WindowClass::~WindowClass()
    {
        UnregisterClassW(m_WindowClassName, m_hInstance);
    }

    Window::Window(uint32_t width, uint32_t height, const wchar_t* title)
    {
        m_Data.Width = width;
        m_Data.Height = height;
        m_Data.Title = title;

        m_Data.EventCallback = [](Event&) -> void {};

        RECT windowRect{ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        if (AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, 0) == FALSE)
        {
            DL_THROW_LAST_WIN32();
        }

        m_hWnd = CreateWindowExW(
            0,
            WindowClass::GetName(),
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
            nullptr, nullptr,
            WindowClass::GetInstance(),
            &m_Data.EventCallback
        );

        if (!m_hWnd)
        {
            DL_THROW_LAST_WIN32();
        }

        ShowWindow(m_hWnd, SW_SHOW);
    }

    Window::~Window()
    {
        DestroyWindow(m_hWnd);
    }

    void Window::OnResize(uint32_t width, uint32_t height)
    {
        m_Data.Width = width;
        m_Data.Height = height;
    }

    LRESULT Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_NCCREATE)
        {
            const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            auto* const eventCallback = static_cast<EventCallbackFn*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(eventCallback));
            SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgInitiate));

            return HandleMsg(hWnd, msg, wParam, lParam, *eventCallback);
        }

        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    LRESULT Window::HandleMsgInitiate(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto* const eventCallback = reinterpret_cast<EventCallbackFn*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        return HandleMsg(hWnd, msg, wParam, lParam, *eventCallback);
    }

    LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, const EventCallbackFn& eventCallback)
    {
        switch (msg)
        {
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            auto windowCloseEvent{ WindowCloseEvent {} };
            eventCallback(windowCloseEvent);
        } return 0;
        case WM_SIZE:
        {
            const auto width = LOWORD(lParam);
            const auto height = HIWORD(lParam);

            auto windowResizeEvent{ WindowResizeEvent { width, height } };
            eventCallback(windowResizeEvent);
        } break;
        case WM_KILLFOCUS:
        {
            Input::ResetKeys();
        } break;

        // Keyboard input
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            if ((HIWORD(lParam) & KF_REPEAT) != KF_REPEAT)
            {
                const auto key = static_cast<uint8_t>(wParam);
                Input::OnKeyPressed(key);

                auto keyPressedEvent{ KeyPressedEvent { key } };
                eventCallback(keyPressedEvent);
            }
        } break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            const auto key = static_cast<uint8_t>(wParam);
            Input::OnKeyReleased(key);

            auto keyReleasedEvent{ KeyReleasedEvent { key } };
            eventCallback(keyReleasedEvent);
        } break;
        // End of keyboard input

        // Mouse input
        case WM_MOUSEWHEEL:
        {
            const auto delta = GET_WHEEL_DELTA_WPARAM(wParam);

            auto mouseWheelEvent{ MouseScrolledEvent { delta } };
            eventCallback(mouseWheelEvent);
        } break;
        case WM_MOUSEMOVE:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            Input::OnMouseMove(pt.x, pt.y);

            auto mouseMovedEvent{ MouseMovedEvent { pt.x, pt.y } };
            eventCallback(mouseMovedEvent);
        } break;
        case WM_LBUTTONDOWN:
        {
            Input::OnKeyPressed(VK_LBUTTON);

            auto mouseButtonPressedEvent{ MouseButtonPressedEvent { VK_LBUTTON } };
            eventCallback(mouseButtonPressedEvent);
        } break;
        case WM_LBUTTONUP:
        {
            Input::OnKeyReleased(VK_LBUTTON);

            auto mouseButtonReleasedEvent{ MouseButtonReleasedEvent { VK_LBUTTON } };
            eventCallback(mouseButtonReleasedEvent);
        } break;
        case WM_RBUTTONDOWN:
        {
            Input::OnKeyPressed(VK_RBUTTON);

            auto mouseButtonPressedEvent{ MouseButtonPressedEvent { VK_RBUTTON } };
            eventCallback(mouseButtonPressedEvent);
        } break;
        case WM_RBUTTONUP:
        {
            Input::OnKeyReleased(VK_RBUTTON);

            auto mouseButtonReleasedEvent{ MouseButtonReleasedEvent { VK_RBUTTON } };
            eventCallback(mouseButtonReleasedEvent);
        } break;
        case WM_MBUTTONDOWN:
        {
            Input::OnKeyPressed(VK_MBUTTON);

            auto mouseButtonPressedEvent{ MouseButtonPressedEvent { VK_MBUTTON } };
            eventCallback(mouseButtonPressedEvent);
        } break;
        case WM_MBUTTONUP:
        {
            Input::OnKeyReleased(VK_MBUTTON);

            auto mouseButtonReleasedEvent{ MouseButtonReleasedEvent { VK_MBUTTON } };
            eventCallback(mouseButtonReleasedEvent);
        } break;
        // End of mouse input
        }

        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}
