#include "dlpch.h"
#include "Application.h"

#include "DLEngine/Core/Events/ApplicationEvent.h"
#include "DLEngine/Core/Events/EventBus.h"
#include "DLEngine/Renderer/Renderer.h"

static void InitConsole()
{
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "conout$", "w", stdout);
    freopen_s(&dummy, "conout$", "w", stderr);
}

Application::Application(const ApplicationSpecification& specification)
    : m_Window(specification.windowWidth, specification.windowHeight, specification.windowTitle)
{
    s_Instance = this;
}

Application::~Application()
{
    EventBus::Get().Shutdown();
}

void Application::Init()
{
    InitConsole();

    m_Scene.Init();

    auto& eventBus = EventBus::Get();
    eventBus.Subscribe<WindowCloseEvent>(DL_BIND_EVENT_CALLBACK(OnWindowClose));
}

void Application::Run()
{
    while (m_IsRunning)
    {
        static const float s_TimeOfOneFrameMS { 1.0f / 60.0f * 1.e3f };

        const float elapsedMS = static_cast<float>(m_Timer.GetElapsedMilliseconds());
        const float dt = elapsedMS - m_LastFrameTime;

        ProcessInputs();
        EventBus::Get().DispatchEvents();

        if (dt > s_TimeOfOneFrameMS)
        {
            std::cout << "dt: " << dt << " ms\n";
            m_LastFrameTime = elapsedMS;

            m_Scene.OnUpdate(dt);
            continue;
        }

        std::this_thread::yield();
    }
}

void Application::ProcessInputs() const
{
    static MSG s_Msg;
    while (PeekMessageW(&s_Msg, m_Window.GetHandle(), 0, 0, PM_REMOVE))
    {
        TranslateMessage(&s_Msg);
        DispatchMessageW(&s_Msg);
    }
}

void Application::OnWindowClose(const WindowCloseEvent& event)
{
    m_IsRunning = false;
}
