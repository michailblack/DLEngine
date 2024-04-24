#pragma once
#include <vector>

#include "DLEngine/Core/Layer.h"

class LayerStack
{
    using LayerStackBuffer          = std::vector<Layer*>;
    using LayerStackIterator        = std::vector<Layer*>::iterator;
    using LayerStackReverseIterator = std::vector<Layer*>::reverse_iterator;

public:
    LayerStack() = default;
    ~LayerStack();

    LayerStack(const LayerStack&) = delete;
    LayerStack(LayerStack&&) = delete;
    LayerStack& operator=(const LayerStack&) = delete;
    LayerStack& operator=(LayerStack&&) = delete;

    void PushLayer(Layer* layer);
    void PopLayer(Layer* layer);

    LayerStackIterator begin() { return m_Layers.begin(); }
    LayerStackIterator end() { return m_Layers.end(); }

    LayerStackReverseIterator rbegin() { return m_Layers.rbegin(); }
    LayerStackReverseIterator rend() { return m_Layers.rend(); }

private:
    LayerStackBuffer m_Layers;
};
