#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/LayerStack.h"
#include "DLEngine/Core/Window.h"

#include "DLEngine/Core/Events/ApplicationEvent.h"
#include "DLEngine/Core/Events/KeyEvent.h"

#include "DLEngine/Utils/Timer.h"

#include <filesystem>

namespace DLEngine
{
    struct ApplicationSpecification
    {
        const wchar_t* WndTitle{ L"New Window" };
        std::filesystem::path WorkingDir{ std::filesystem::current_path() };
        uint32_t WndWidth{ 800 };
        uint32_t WndHeight{ 600 };
    };

    class Application
    {
    public:
        ~Application();

        Application(const Application&) = delete;
        Application(Application&&) = delete;
        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) = delete;

        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        Window& GetWindow() noexcept { return *m_Window; }
        static Application& Get() noexcept { return *s_Instance; }

        const std::filesystem::path& GetWorkingDir() const noexcept { return m_Specification.WorkingDir; }

    protected:
        explicit Application(const ApplicationSpecification& spec);

    private:
        bool OnWindowClose(WindowCloseEvent&);
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);

    private:
        inline static Application* s_Instance{ nullptr };
        inline static constexpr float s_TimeOfOneFrameMS{ 1.0f / 60.0f * 1.e3f };

        ApplicationSpecification m_Specification;

        bool m_IsRunning{ true };

        Scope<Window> m_Window;
        LayerStack m_LayerStack{};

        Timer m_Timer;
    };

    extern Application* CreateApplication(std::wstring_view cmdLine);
}
