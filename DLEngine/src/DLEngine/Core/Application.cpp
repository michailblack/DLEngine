﻿#include "dlpch.h"
#include "Application.h"

#include "DLEngine/Core/Engine.h"
#include "DLEngine/Core/ImGuiLayer.h"

#include "DLEngine/Renderer/Renderer.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"

#include "DLEngine/Utils/DeltaTime.h"

namespace DLEngine
{
    Application::~Application()
    {
        for (const auto& layer : m_LayerStack)
            layer->OnDetach();

        Engine::Deinit();
    }

    void Application::Run()
    {
        while (m_IsRunning)
        {
            DeltaTime dt{ static_cast<float>(m_Timer.GetDeltaTimeMS()) };

            ProcessInputs();

            if (m_Timer.FrameElapsed(dt))
            {
                m_Timer.Reset();

                Renderer::BeginFrame(dt);

                for (const auto& layer : m_LayerStack)
                    layer->OnUpdate(dt);

                ImGuiLayer::Begin();
                for (const auto& layer : m_LayerStack)
                    layer->OnImGuiRender();
                ImGuiLayer::End();

                Renderer::EndFrame();
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
        , m_Window(CreateScope<Window>(spec.WndWidth, spec.WndHeight, spec.WndTitle))
        , m_Timer(s_TimeOfOneFrameMS)
    {
        s_Instance = this;
        m_Window->SetEventCallback(DL_BIND_EVENT_FN(Application::OnEvent));

        Engine::Init();

        PushOverlay(new ImGuiLayer());
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

    bool Application::OnWindowClose(WindowCloseEvent&)
    {
        m_IsRunning = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        m_Window->OnResize(e.GetWidth(), e.GetHeight());
        Renderer::OnResize(e.GetWidth(), e.GetHeight());

        return false;
    }

    bool Application::OnKeyPressed(KeyPressedEvent& e)
    {
        switch (e.GetKeyCode())
        {
        case VK_ESCAPE:
            m_IsRunning = false;
            break;
        case 'N':
            MeshSystem::Get().ToggleNormalVis();
            break;
        default:
            break;
        }

        return false;
    }
}
