#include "WorldLayer.h"

#include "DLEngine/DirectX/D3D.h"

#include "DLEngine/Mesh/ModelManager.h"
#include "DLEngine/Mesh/NormalVisGroup.h"
#include "DLEngine/Mesh/HologramGroup.h"

#include "DLEngine/Renderer/Renderer.h"

WorldLayer::WorldLayer()
    : m_CameraController(DLEngine::Camera { DLEngine::Math::ToRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    const auto cube{ DLEngine::ModelManager::Load(R"(..\models\cube\cube.obj)") };

    DLEngine::NormalVisGroupInstance normalVisGroupInstance{};
    normalVisGroupInstance.Transform = DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 2.0f, 0.0f, 3.0f });

    DLEngine::Renderer::Submit(cube, DLEngine::NormalVisGroupMaterial{}, normalVisGroupInstance);

    DLEngine::HologramGroupInstance hologramGroupInstance{};
    hologramGroupInstance.Transform = DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 0.0f, 5.0f, 8.0f });
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 1.0f, 0.0f, 1.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 0.0f, 1.0f, 1.0f };

    DLEngine::Renderer::Submit(cube, DLEngine::HologramGroupMaterial{}, hologramGroupInstance);

    hologramGroupInstance.Transform = DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -3.0f, 0.0f, 0.0f });
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 0.0f, 0.0f, 1.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 1.0f, 1.0f, 0.0f };

    DLEngine::Renderer::Submit(cube, DLEngine::HologramGroupMaterial{}, hologramGroupInstance);
}

void WorldLayer::OnDetach()
{
    
}

void WorldLayer::OnUpdate(DeltaTime dt)
{
    DLEngine::Renderer::BeginScene(m_CameraController.GetCamera());

    m_CameraController.OnUpdate(dt);

    DLEngine::Renderer::EndScene();
}

void WorldLayer::OnEvent(DLEngine::Event& e)
{
    m_CameraController.OnEvent(e);
}
