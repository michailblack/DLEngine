#include "dlpch.h"
#include "Application.h"

#include "DLEngine/Renderer/Renderer.h"

Application::Application(const ApplicationSpecification& specification)
    : m_Specification(specification)
    , m_Timer(s_TimeOfOneFrameMS)
{
    s_Instance = this;
}

Application::~Application()
{
}

void Application::Init()
{
    InitConsole();

    m_Window = CreateScope<Window>(m_Specification.WndWidth, m_Specification.WndHeight, m_Specification.WndTitle);
    Renderer::SetWindow(m_Window);

    m_Scene.Init();
}

void Application::Run()
{
    MSG msg;

    while (!m_Window->ShouldClose())
    {
        const float dt = static_cast<float>(m_Timer.GetDeltaTimeMS());

        ZeroMemory(&msg, sizeof(MSG));
        ProcessInputs(msg);

        if (m_Timer.FrameElapsed(dt))
        {
            m_Timer.Reset();
            m_Scene.OnUpdate(dt);
        }

        std::this_thread::yield();
    }
}

void Application::InitConsole()
{
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "conout$", "w", stdout);
    freopen_s(&dummy, "conout$", "w", stderr);
}

void Application::ProcessInputs(MSG& msg) const
{
    while (PeekMessageW(&msg, m_Window->GetHandle(), 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}
