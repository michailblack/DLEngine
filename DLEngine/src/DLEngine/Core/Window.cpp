#include "dlpch.h"
#include "Window.h"

#include "DLEngine/Core/DLException.h"
#include "DLEngine/Core/Input.h"

#include "DLEngine/DirectX/D3D.h"

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
        swapChainDesk.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesk.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesk.Flags = 0;

        DL_THROW_IF_HR(D3D::GetFactory7()->CreateSwapChainForHwnd(
            D3D::GetDevice5().Get(),
            m_hWnd,
            &swapChainDesk,
            nullptr,
            nullptr,
            &m_Data.SwapChain
        ));

        DL_THROW_IF_HR(m_Data.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D1), &m_Data.BackBuffer));
        m_Data.BackBuffer->GetDesc1(&m_Data.BackBufferDesc);

        m_Data.DepthStencilDesk.Width = m_Data.BackBufferDesc.Width;
        m_Data.DepthStencilDesk.Height = m_Data.BackBufferDesc.Height;
        m_Data.DepthStencilDesk.MipLevels = 1;
        m_Data.DepthStencilDesk.ArraySize = 1;
        m_Data.DepthStencilDesk.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        m_Data.DepthStencilDesk.SampleDesc.Count = 1;
        m_Data.DepthStencilDesk.SampleDesc.Quality = 0;
        m_Data.DepthStencilDesk.Usage = D3D11_USAGE_DEFAULT;
        m_Data.DepthStencilDesk.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        m_Data.DepthStencilDesk.CPUAccessFlags = 0;
        m_Data.DepthStencilDesk.MiscFlags = 0;
        m_Data.DepthStencilDesk.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateTexture2D1(&m_Data.DepthStencilDesk, nullptr, &m_Data.DepthStencil));

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateRenderTargetView1(m_Data.BackBuffer.Get(), nullptr, &m_Data.BackBufferView));
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateDepthStencilView(m_Data.DepthStencil.Get(), nullptr, &m_Data.DepthStencilView));

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        D3D::GetDeviceContext4()->RSSetViewports(1, &viewport);

        ShowWindow(m_hWnd, SW_SHOW);

        DL_LOG_INFO("Created window ({:d}, {:d})", width, height);
    }

    Window::~Window()
    {
        DestroyWindow(m_hWnd);
    }

    void Window::Present() const
    {
        DL_THROW_IF_HR(m_Data.SwapChain->Present(1, 0));
    }

    void Window::OnResize(uint32_t width, uint32_t height)
    {
        m_Data.Width = width;
        m_Data.Height = height;

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        D3D::GetDeviceContext4()->RSSetViewports(1, &viewport);

        D3D::GetDeviceContext4()->OMSetRenderTargets(0, nullptr, nullptr);

        m_Data.BackBuffer.Reset();
        m_Data.DepthStencil.Reset();
        m_Data.BackBufferView.Reset();
        m_Data.DepthStencilView.Reset();

        DL_THROW_IF_HR(m_Data.SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

        DL_THROW_IF_HR(m_Data.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D1), &m_Data.BackBuffer));
        m_Data.BackBuffer->GetDesc1(&m_Data.BackBufferDesc);

        m_Data.DepthStencilDesk.Width = m_Data.BackBufferDesc.Width;
        m_Data.DepthStencilDesk.Height = m_Data.BackBufferDesc.Height;

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateTexture2D1(&m_Data.DepthStencilDesk, nullptr, &m_Data.DepthStencil));

        DL_THROW_IF_HR(D3D::GetDevice5()->CreateRenderTargetView1(m_Data.BackBuffer.Get(), nullptr, &m_Data.BackBufferView));
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateDepthStencilView(m_Data.DepthStencil.Get(), nullptr, &m_Data.DepthStencilView));
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
