#pragma once
#include "DLEngine/Core/Layer.h"

namespace DLEngine
{
    class ImGuiLayer
        : public Layer
    {
    public:
        ImGuiLayer() = default;
        ~ImGuiLayer() override = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(DeltaTime) override;
        void OnImGuiRender() override;
        void OnEvent(Event& e) override;

        static void Begin();
        static void End();
    };
}
