#pragma once
#include "DLEngine/Core/Mouse.h"
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Keyboard.h"
#include "DLEngine/Core/Window.h"
#include "DLEngine/Renderer/Scene.h"
#include "DLEngine/Utils/Timer.h"

struct ApplicationSpecification
{
    uint32_t WndWidth { 800 };
    uint32_t WndHeight { 600 };
    const wchar_t* WndTitle { L"New Window" };
};

class Application
{
    friend int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow);
public:
    ~Application();

    Application (const Application&) = delete;
    Application (Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    static Application& Get() { return *s_Instance; }

    Ref<Window>& GetWindow() { return m_Window; }

    void Init();
    void Run();

protected:
    explicit Application(const ApplicationSpecification& specification);

private:
    static void InitConsole();

    void ProcessInputs(MSG& msg) const;

private:
    inline static Application* s_Instance { nullptr };
    inline static constexpr float s_TimeOfOneFrameMS { 1.0f / 60.0f * 1.e3f };

    ApplicationSpecification m_Specification;

    Ref<Window> m_Window;

    Scene m_Scene;

    Timer m_Timer;
};
