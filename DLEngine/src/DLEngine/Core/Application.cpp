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

void Application::OnApplicationRender(const ApplicationRenderEvent& event)
{
    Renderer::Clear({ 0, 0, 0 });
    Renderer::RenderScene(m_Scene);
    Renderer::SwapFramebuffer();
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
    eventBus.Subscribe<ApplicationRenderEvent>(DL_BIND_EVENT_CALLBACK(OnApplicationRender));
}

void Application::Run()
{
    while (m_IsRunning)
    {
        const float elapsedMS = static_cast<float>(m_Timer.GetElapsedMilliseconds());
        const float dt = elapsedMS - m_LastFrameTime;
        m_LastFrameTime = elapsedMS;

        m_Scene.OnUpdate(dt);

        ProcessInputs(); 
        EventBus::Get().DispatchEvents();

        while (m_Timer.GetElapsedMilliseconds() - elapsedMS < 16.7)
            std::this_thread::yield();
    }
}

void Application::ProcessInputs() const
{
    static MSG s_Msg;
    if (PeekMessageW(&s_Msg, m_Window.GetHandle(), 0, 0, PM_REMOVE))
    {
        TranslateMessage(&s_Msg);
        DispatchMessageW(&s_Msg);
    }
}

void Application::OnWindowClose(const WindowCloseEvent& event)
{
    m_IsRunning = false;
}
