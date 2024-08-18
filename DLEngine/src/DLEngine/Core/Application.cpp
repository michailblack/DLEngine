#include "dlpch.h"
#include "Application.h"

#include "DLEngine/Core/ImGuiLayer.h"

#include "DLEngine/Math/Math.h"

#include "DLEngine/Renderer/Renderer.h"

#include "DLEngine/Utils/DeltaTime.h"

#include <DirectXTex/DirectXTex.h>

namespace DLEngine
{
    namespace
    {
        void InitConsole()
        {
            AllocConsole();
            FILE* dummy;
            freopen_s(&dummy, "conout$", "w", stdout);
            freopen_s(&dummy, "conout$", "w", stderr);
        }
    }

    Application::~Application()
    {
        for (const auto& layer : m_LayerStack)
            layer->OnDetach();
    }

    void Application::Run()
    {
        while (m_IsRunning)
        {
            DeltaTime dt{ static_cast<float>(m_Timer.GetDeltaTimeMS()) };

            m_Window->ProcessEvents();

            if (m_Timer.FrameElapsed(dt))
            {
                m_Timer.Reset();

                Renderer::BeginFrame();

                for (const auto& layer : m_LayerStack)
                    layer->OnUpdate(dt);

                ImGuiLayer::Begin();
                for (const auto& layer : m_LayerStack)
                    layer->OnImGuiRender();
                ImGuiLayer::End();

                Renderer::EndFrame();

                m_Window->SwapBuffers();
            }

            std::this_thread::yield();
        }
    }

    void Application::OnEvent(Event& e)
    {
        try
        {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<WindowCloseEvent>(DL_BIND_EVENT_FN(Application::OnWindowClose));
            dispatcher.Dispatch<WindowResizeEvent>(DL_BIND_EVENT_FN(Application::OnWindowResize));
            dispatcher.Dispatch<KeyPressedEvent>(DL_BIND_EVENT_FN(Application::OnKeyPressed));

            for (const auto& layer : m_LayerStack | std::views::reverse)
            {
                if (e.Handled)
                    break;

                layer->OnEvent(e);
            }
        }
        catch (const DLException& e)
        {
            MessageBoxExA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION, 0);
            m_IsRunning = false;
        }
        catch (const std::exception& e)
        {
            MessageBoxExA(nullptr, e.what(), "Standard exception", MB_OK | MB_ICONEXCLAMATION, 0);
            m_IsRunning = false;
        }
        catch (...)
        {
            MessageBoxExA(nullptr, "Unknown exception", "Unknown exception", MB_OK | MB_ICONEXCLAMATION, 0);
            m_IsRunning = false;
        }
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    Application::Application(const ApplicationSpecification& spec)
        : m_Specification(spec)
        , m_Timer(s_TimeOfOneFrameMS)
    {
        s_Instance = this;

        InitConsole();

        Log::Init();

        if (!DirectX::XMVerifyCPUSupport())
            throw std::runtime_error{ "DirectXMath Library does not support the given platform" };
        DL_THROW_IF_HR(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

        m_Window = CreateScope<Window>(m_Specification.WndWidth, m_Specification.WndHeight, m_Specification.WndTitle);
        m_Window->SetEventCallback(DL_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();

        PushOverlay(new ImGuiLayer());
    }

    bool Application::OnWindowClose(WindowCloseEvent&)
    {
        m_IsRunning = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        Renderer::InvalidateSwapChainTargetFramebuffer();
        m_Window->OnResize(e.GetWidth(), e.GetHeight());
        Renderer::RecreateSwapChainTargetFramebuffer();

        return false;
    }

    bool Application::OnKeyPressed(KeyPressedEvent& e)
    {
        switch (e.GetKeyCode())
        {
        case VK_ESCAPE:
            m_IsRunning = false;
            break;
        default:
            break;
        }

        return false;
    }
}
