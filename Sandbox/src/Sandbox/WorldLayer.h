#pragma once
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Renderer/CameraController.h"
#include "DLEngine/Renderer/Entity.h"

#include "DLEngine/DirectX/ConstantBuffers.h"
#include "DLEngine/DirectX/InputLayout.h"
#include "DLEngine/DirectX/Shaders.h"
#include "DLEngine/DirectX/VertexBuffer.h"

class WorldLayer
    : public DLEngine::Layer
{
public:
    WorldLayer();
    ~WorldLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(DeltaTime dt) override;
    void OnEvent(DLEngine::Event& e) override;

private:
    DLEngine::CameraController m_CameraController;
};
