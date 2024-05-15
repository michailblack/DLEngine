#pragma once
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Renderer/CameraController.h"
#include "DLEngine/Renderer/Entity.h"

#include "DLEngine/DirectX/Shaders.h"
#include "DLEngine/DirectX/ConstantBuffers.h"
#include "DLEngine/DirectX/VertexBuffer.h"
#include "DLEngine/DirectX/InputLayout.h"

class WorldLayer
    : public Layer
{
public:
    WorldLayer();
    ~WorldLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float dt) override;
    void OnEvent(Event& e) override;

private:
    void DrawTestTriangle();

private:
    CameraController m_CameraController;

    struct
    {
        Math::Vec2 Resolution { 0.0f };
        float Time { 0.0f };
        uint8_t _padding[4];
    } m_PerFrameData;

    Ref<VertexBuffer> m_VertexBuffer;
    //Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
    Ref<InputLayout> m_InputLayout;
    Ref<VertexShader> m_VertexShader;
    Ref<PixelShader> m_PixelShader;
    Ref<PixelConstantBuffer<decltype(m_PerFrameData)>> m_ConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState2> m_RasterizerState;
};

