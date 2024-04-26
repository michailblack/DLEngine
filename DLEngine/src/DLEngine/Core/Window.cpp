#include "dlpch.h"
#include "Window.h"

#include "DLEngine/Core/DLException.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/Core/D3D.h"
#include "DLEngine/Core/Events/ApplicationEvent.h"
#include "DLEngine/Core/Events/KeyEvent.h"
#include "DLEngine/Core/Events/MouseEvent.h"

#include "DLEngine/Renderer/Renderer.h"

#pragma comment(lib, "dxgi.lib")

Window::WindowClass::WindowClass()
{
    if (GetModuleHandleExW(0, nullptr, &m_hInstance) == 0)
    {
        throw DL_LAST_ERROR();
    }

    WNDCLASSEXW wc {};
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

Window::Window(uint32_t width, uint32_t height, const wchar_t* title, const EventCallbackFn& callback)
{
    m_Data.m_Width = width;
    m_Data.m_Height = height;
    m_Data.m_Title = title;
    m_Data.m_EventCallback = callback;

    RECT windowRect { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    if (AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, 0) == FALSE)
    {
        throw DL_LAST_ERROR();
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
        this
    );

    if (!m_hWnd)
    {
        throw DL_LAST_ERROR();
    }

    Microsoft::WRL::ComPtr<IDXGIFactory2> idxgiFactory2;
    DL_THROW_IF(CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), &idxgiFactory2));
    
    DXGI_SWAP_CHAIN_DESC1 swapChainDesk {};
    swapChainDesk.Width = width;
    swapChainDesk.Height = height;
    swapChainDesk.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesk.Stereo = FALSE;
    swapChainDesk.SampleDesc.Count = 1;
    swapChainDesk.SampleDesc.Quality = 0;
    swapChainDesk.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesk.BufferCount = 2;
    swapChainDesk.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesk.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesk.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesk.Flags = 0;

    DL_THROW_IF(idxgiFactory2->CreateSwapChainForHwnd(
        D3D::Get().GetDevice().Get(),
        m_hWnd,
        &swapChainDesk,
        nullptr,
        nullptr,
        &m_Data.m_SwapChain
    ));

    Microsoft::WRL::ComPtr<ID3D11Texture2D1> backBuffer;
    DL_THROW_IF(m_Data.m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D1), &backBuffer));

    D3D::Get().GetDevice()->CreateRenderTargetView1(backBuffer.Get(), nullptr, &m_Data.m_RenderTargetView);

    ShowWindow(m_hWnd, SW_SHOW);
}

Window::~Window()
{
    DestroyWindow(m_hWnd);
}

void Window::Present() const
{
    DL_THROW_IF(m_Data.m_SwapChain->Present(1, 0));
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
            auto windowCloseEvent { WindowCloseEvent {} };
            m_Data.m_EventCallback(windowCloseEvent);
        } return 0;
    case WM_SIZE:
        {
            const auto width = LOWORD(lParam);
            const auto height = HIWORD(lParam);

            m_Data.m_Width = width;
            m_Data.m_Height = height;

            Renderer::OnResize(width, height);

            auto windowResizeEvent { WindowResizeEvent { width, height } };
            m_Data.m_EventCallback(windowResizeEvent);
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

                auto keyPressedEvent { KeyPressedEvent { key } };
                m_Data.m_EventCallback(keyPressedEvent);
            }
        } break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        {
            const auto key = static_cast<uint8_t>(wParam);
            Input::OnKeyReleased(key);

            auto keyReleasedEvent { KeyReleasedEvent { key } };
            m_Data.m_EventCallback(keyReleasedEvent);
        } break;
    // End of keyboard input

    // Mouse input
    case WM_MOUSEWHEEL:
        {
            const auto delta = GET_WHEEL_DELTA_WPARAM(wParam);

            auto mouseWheelEvent { MouseScrolledEvent { delta } };
            m_Data.m_EventCallback(mouseWheelEvent);
        } break;
    case WM_MOUSEMOVE:
        {
            const POINTS pt = MAKEPOINTS(lParam);
            Input::OnMouseMove(pt.x, pt.y);

            auto mouseMovedEvent { MouseMovedEvent { pt.x, pt.y } };
            m_Data.m_EventCallback(mouseMovedEvent);
        } break;
    case WM_LBUTTONDOWN:
        {
            Input::OnKeyPressed(VK_LBUTTON);

            auto mouseButtonPressedEvent { MouseButtonPressedEvent { VK_LBUTTON } };
            m_Data.m_EventCallback(mouseButtonPressedEvent);
        } break;
    case WM_LBUTTONUP:
        {
            Input::OnKeyReleased(VK_LBUTTON);

            auto mouseButtonReleasedEvent { MouseButtonReleasedEvent { VK_LBUTTON } };
            m_Data.m_EventCallback(mouseButtonReleasedEvent);
        } break;
    case WM_RBUTTONDOWN:
        {
            Input::OnKeyPressed(VK_RBUTTON);

            auto mouseButtonPressedEvent { MouseButtonPressedEvent { VK_RBUTTON } };
            m_Data.m_EventCallback(mouseButtonPressedEvent);
        } break;
    case WM_RBUTTONUP:
        {
            Input::OnKeyReleased(VK_RBUTTON);

            auto mouseButtonReleasedEvent { MouseButtonReleasedEvent { VK_RBUTTON } };
            m_Data.m_EventCallback(mouseButtonReleasedEvent);
        } break;
    case WM_MBUTTONDOWN:
        {
            Input::OnKeyPressed(VK_MBUTTON);

            auto mouseButtonPressedEvent { MouseButtonPressedEvent { VK_MBUTTON } };
            m_Data.m_EventCallback(mouseButtonPressedEvent);
        } break;
    case WM_MBUTTONUP:
        {
            Input::OnKeyReleased(VK_MBUTTON);

            auto mouseButtonReleasedEvent { MouseButtonReleasedEvent { VK_MBUTTON } };
            m_Data.m_EventCallback(mouseButtonReleasedEvent);
        } break;
    // End of mouse input
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
