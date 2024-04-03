#pragma once
#include "DLEngine/Core/Window.h"
#include "DLEngine/Renderer/Scene.h"
#include "DLEngine/Utils/Timer.h"

struct ApplicationSpecification
{
    uint32_t windowWidth { 800 };
    uint32_t windowHeight { 600 };
    const wchar_t* windowTitle { L"New Window" };
};

class Application
{
public:
    ~Application();

    Application (const Application&) = delete;
    Application (Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    static Application* Get() { return s_Instance; }
    Window& GetWindow() { return m_Window; }

    void Init();
    void Run();

protected:
    explicit Application(const ApplicationSpecification& specification);

private:
    inline static Application* s_Instance { nullptr };

private:
    void ProcessInputs() const;

private:
    void OnWindowClose(const WindowCloseEvent& event);

private:
    Window m_Window;
    Scene m_Scene;

    Timer m_Timer;
    float m_LastFrameTime { 0.0f };

    bool m_IsRunning { true };
};

extern Application* CreateApplication();
