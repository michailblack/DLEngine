﻿#include "dlpch.h"
#include "Application.h"

#include "DLEngine/Renderer/RenderCommand.h"
#include "DLEngine/Renderer/Renderer.h"

Application::~Application()
{
    for (const auto& layer : m_LayerStack)
        layer->OnDetach();
}

void Application::Run()
{
    while (m_IsRunning)
    {
        const float dt = static_cast<float>(m_Timer.GetDeltaTimeMS());

        ProcessInputs();

        if (m_Timer.FrameElapsed(dt))
        {
            m_Timer.Reset();

            for (const auto& layer : m_LayerStack)
                layer->OnUpdate(dt);

            m_Window->Present();
        }

        std::this_thread::yield();
    }
}

void Application::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(DL_BIND_EVENT_FN(Application::OnWindowClose));
    dispatcher.Dispatch<KeyPressedEvent>(DL_BIND_EVENT_FN(Application::OnKeyPressed));

    for (const auto& layer : m_LayerStack | std::views::reverse)
    {
        if (e.Handled)
            break;

        layer->OnEvent(e);
    }
}

void Application::PushLayer(Layer* layer)
{
    m_LayerStack.PushLayer(layer);
    layer->OnAttach();
}

Application::Application(const ApplicationSpecification& spec)
    : m_Specification(spec)
    , m_Timer(s_TimeOfOneFrameMS)
{
    s_Instance = this;

    InitConsole();

    m_Window = CreateScope<Window>(spec.WndWidth, spec.WndHeight, spec.WndTitle, DL_BIND_EVENT_FN(Application::OnEvent));
    Renderer::Init();
}

void Application::InitConsole()
{
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "conout$", "w", stdout);
    freopen_s(&dummy, "conout$", "w", stderr);
}

void Application::ProcessInputs() const
{
    MSG msg;
    while (PeekMessageW(&msg, m_Window->GetHandle(), 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
    m_IsRunning = false;
    return true;
}

bool Application::OnKeyPressed(KeyPressedEvent& e)
{
    switch (e.GetKeyCode())
    {
    case VK_ESCAPE:
        m_IsRunning = false;
        break;
    case '1':
    {
        Renderer::SetFramebufferSizeCoefficient(1);

        AppRenderEvent e {};
        OnEvent(e);
    } break;
    case '2':
    {
        Renderer::SetFramebufferSizeCoefficient(3);
        
        AppRenderEvent e{};
        OnEvent(e);
    } break;
    case '3':
    {
        Renderer::SetFramebufferSizeCoefficient(6);

        AppRenderEvent e{};
        OnEvent(e);
    } break;
    case '4':
    {
        Renderer::SetFramebufferSizeCoefficient(9);

        AppRenderEvent e{};
        OnEvent(e);
    } break;
    default:
        break;
    }

    return false;
}
