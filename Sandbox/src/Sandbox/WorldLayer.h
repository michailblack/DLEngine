#pragma once
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Core/Events/ApplicationEvent.h"

#include "DLEngine/Renderer/CameraController.h"
#include "DLEngine/Renderer/Entity.h"

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

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState2> m_RasterizerState;
};

