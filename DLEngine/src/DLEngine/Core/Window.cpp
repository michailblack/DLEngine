#include "dlpch.h"
#include "Window.h"

#include <cassert>

#include "DLEngine/Entity/Entity.h"
#include "Events/ApplicationEvent.h"
#include "Events/EventBus.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto& eventBus = EventBus::Get();

    switch (msg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            eventBus.TriggerEvent(WindowCloseEvent {});
            return 0;
        case WM_SIZE:
        {
            const auto width = LOWORD(lParam);
            const auto height = HIWORD(lParam);
            eventBus.TriggerEvent(WindowResizeEvent { width, height });
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        }
        case WM_PAINT:
        {
            EventBus::Get().TriggerEvent(ApplicationRenderEvent {});
            break;
        }
        case WM_KEYDOWN:
        {
            eventBus.QueueEvent(KeyPressedEvent { static_cast<int32_t>(wParam), 0 });
            break;
        }
        case WM_RBUTTONDOWN:
        {
            const POINTS points = MAKEPOINTS(lParam);
            eventBus.QueueEvent(MouseButtonPressedEvent { MouseButton::Right, wParam, points.x, points.y });
            break;
        }
        case WM_RBUTTONUP:
        {
            const POINTS points = MAKEPOINTS(lParam);
            eventBus.QueueEvent(MouseButtonReleasedEvent { MouseButton::Right, wParam, points.x, points.y });
            break;
        }
        case WM_MOUSEMOVE:
        {
            const POINTS points = MAKEPOINTS(lParam);
            eventBus.QueueEvent(MouseMovedEvent { wParam, points.x, points.y });
            break;
        }
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

Window::WindowClass::WindowClass()
{
    assert(GetModuleHandleExW(0, nullptr, &m_hInstance));

    WNDCLASSEXW wc {};
    memset(&wc, 0, sizeof(wc));

    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
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
    : m_Width(width), m_Height(height), m_Title(title)
{
    EventBus::Get().Subscribe<WindowResizeEvent>(DL_BIND_EVENT_CALLBACK(OnWindowResizeEvent));

    m_BitmapFramebuffer.resize(m_Width * m_Height / 4);

    RECT windowRect { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };
    AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, 0);

    m_hWnd = CreateWindowExW(
        0,
        WindowClass::GetName(),
        m_Title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        nullptr, nullptr,
        WindowClass::GetInstance(),
        nullptr
    );

    assert(m_hWnd);

    ShowWindow(m_hWnd, SW_SHOW);
}

Window::~Window()
{
    DestroyWindow(m_hWnd);
}

void Window::OnWindowResizeEvent(const WindowResizeEvent& event)
{
    m_Width = event.GetWidth();
    m_Height = event.GetHeight();

    m_BitmapFramebuffer.resize(m_Width * m_Height / 4);
}
