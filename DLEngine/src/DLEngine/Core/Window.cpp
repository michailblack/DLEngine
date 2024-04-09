#include "dlpch.h"
#include "Window.h"

#include <cassert>

#include "DLEngine/Renderer/Renderer.h"

Window::WindowClass::WindowClass()
{
    assert(GetModuleHandleExW(0, nullptr, &m_hInstance));

    WNDCLASSEXW wc {};
    memset(&wc, 0, sizeof(wc));

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
    : m_Width(width), m_Height(height), m_Title(title)
{
    m_BitmapFramebuffer.resize(GetFramebufferSize().Data[0] * GetFramebufferSize().Data[1]);

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
        this
    );

    assert(m_hWnd);

    ShowWindow(m_hWnd, SW_SHOW);
}

Window::~Window()
{
    DestroyWindow(m_hWnd);
}

void Window::OnResize(uint32_t width, uint32_t height)
{
    m_Width = width;
    m_Height = height;

    m_BitmapFramebuffer.resize(GetFramebufferSize().Data[0] * GetFramebufferSize().Data[1]);
}

LRESULT Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        Window* const wndHandle = static_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wndHandle));
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgPassToWndMember));

        return wndHandle->HandleMsg(hWnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsgPassToWndMember(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* const wndHandle = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    return wndHandle->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        {
            PostQuitMessage(0);
            m_ShouldClose = true;
        } return 0;
    case WM_SIZE:
        {
            const auto width = LOWORD(lParam);
            const auto height = HIWORD(lParam);
            OnResize(width, height);
            m_ShouldRedraw = true;
        } break;
    case WM_KILLFOCUS:
        {
            Keyboard.ResetKeys();
        } break;

    // Keyboard input
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        {
            Keyboard.OnKeyPressed(static_cast<uint8_t>(wParam));
        } break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        {
            Keyboard.OnKeyReleased(static_cast<uint8_t>(wParam));
        } break;
    // End of keyboard input

    // Mouse input
    case WM_MOUSEMOVE:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            Mouse.OnMouseMove(pt.x, pt.y);
        } break;
    case WM_LBUTTONDOWN:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            Mouse.OnMouseButtonPressed(Mouse::Button::Left, pt.x, pt.y);
        } break;
    case WM_LBUTTONUP:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            Mouse.OnMouseButtonReleased(Mouse::Button::Left, pt.x, pt.y);
        } break;
    case WM_RBUTTONDOWN:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            Mouse.OnMouseButtonPressed(Mouse::Button::Right, pt.x, pt.y);
        } break;
    case WM_RBUTTONUP:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            Mouse.OnMouseButtonReleased(Mouse::Button::Right, pt.x, pt.y);
        } break;
    case WM_MBUTTONDOWN:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            Mouse.OnMouseButtonPressed(Mouse::Button::Middle, pt.x, pt.y);
        } break;
    case WM_MBUTTONUP:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            Mouse.OnMouseButtonReleased(Mouse::Button::Middle, pt.x, pt.y);
        } break;
    // End of mouse input
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
