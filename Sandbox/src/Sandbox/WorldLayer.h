#pragma once
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Renderer/CameraController.h"
#include "DLEngine/Renderer/Entity.h"
#include "DLEngine/Renderer/Model.h"

#include "DLEngine/DirectX/Shaders.h"
#include "DLEngine/DirectX/ConstantBuffers.h"
#include "DLEngine/DirectX/InputLayout.h"

template <typename T>
class DLEngine::PerVertexBuffer;

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
    void DrawTestTriangle();

private:
    DLEngine::CameraController m_CameraController;

    struct
    {
        DLEngine::Math::Vec2 Resolution { 0.0f };
        float Time { 0.0f };
        uint8_t _padding[4] {};
    } m_PerFrameData;

    struct Vertex
    {
        DLEngine::Math::Vec3 Position;
        DLEngine::Math::Vec3 Color;
    };

    Ref<DLEngine::PerVertexBuffer<Vertex>> m_VertexBuffer;
    Ref<DLEngine::InputLayout> m_InputLayout;
    Ref<DLEngine::VertexShader> m_VertexShader;
    Ref<DLEngine::PixelShader> m_PixelShader;
    Ref<DLEngine::PixelConstantBuffer<decltype(m_PerFrameData)>> m_ConstantBuffer;
    Ref<DLEngine::Model> m_Cube;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState2> m_RasterizerState;
};
