#pragma once
#include "DLEngine/Core/Events/Event.h"

#include "DLEngine/Utils/DeltaTime.h"

namespace DLEngine
{
    class Layer
    {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        Layer(const Layer&) = delete;
        Layer(Layer&&) = delete;
        Layer& operator=(const Layer&) = delete;
        Layer& operator=(Layer&&) = delete;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(DeltaTime) {}
        virtual void OnEvent(Event&) {}
    };
}
