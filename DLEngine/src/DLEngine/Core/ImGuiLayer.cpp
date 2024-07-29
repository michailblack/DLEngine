#include "dlpch.h"
#include "ImGuiLayer.h"

#include "DLEngine/Core/Application.h"

#include "DLEngine/DirectX/D3D.h"

#define IMGUI_IMPLEMENTATION
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/imgui.cpp>
#include <imgui/imgui_impl_win32.cpp>
#include <imgui/imgui_impl_dx11.cpp>
#include <imgui/imgui_demo.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_tables.cpp>
#include <imgui/imgui_widgets.cpp>

namespace DLEngine
{
    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(Application::Get().GetWindow()->GetHandle());
        ImGui_ImplDX11_Init(D3D::GetDevice5().Get(), D3D::GetDeviceContext4().Get());
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnUpdate(DeltaTime)
    {
    }

    void ImGuiLayer::OnImGuiRender()
    {
    }

    void ImGuiLayer::OnEvent(Event& e)
    {
        ImGuiIO& io = ImGui::GetIO();
        e.Handled |= io.WantCaptureMouse || io.WantCaptureKeyboard;
    }

    void ImGuiLayer::Begin()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End()
    {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}