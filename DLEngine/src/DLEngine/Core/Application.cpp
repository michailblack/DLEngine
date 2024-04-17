#include "dlpch.h"
#include "Application.h"

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
        }

        std::this_thread::yield();
    }
}

void Application::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(DL_BIND_EVENT_FN(Application::OnWindowClose));

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
