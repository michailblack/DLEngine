#include "dlpch.h"
#include "LayerStack.h"

namespace DLEngine
{
    LayerStack::~LayerStack()
    {
        for (const Layer* layer : m_Layers)
            delete layer;
    }

    void LayerStack::PushLayer(Layer* layer)
    {
        m_Layers.emplace_back(layer);
    }

    void LayerStack::PopLayer(Layer* layer)
    {
        const auto it = std::ranges::find(m_Layers, layer);
        if (it != m_Layers.end())
            m_Layers.erase(it);
    }
}
