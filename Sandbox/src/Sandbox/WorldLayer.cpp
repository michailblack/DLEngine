#include "WorldLayer.h"

#include "DLEngine/Core/DLException.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/VertexBuffer.h"

#include "DLEngine/Renderer/ModelManager.h"
#include "DLEngine/Renderer/Renderer.h"
#include "DLEngine/Renderer/NormalVisGroup.h"

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

    const std::vector<DLEngine::NormalVisGroup::Instance> normalVisGroupInstances{
        { DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{  2.0f, 0.0f, 3.0f }) },
    };
    DLEngine::Renderer::SubmitToNormalVisGroup(cube, normalVisGroupInstances);

    const std::vector<DLEngine::HologramGroup::Instance> hologramGroupInstances{
        {
            DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{  0.0f, 5.0f, 8.0f }),
            DLEngine::Math::Vec3{ 1.0f, 0.0f, 1.0f },
            DLEngine::Math::Vec3{ 0.0f, 1.0f, 1.0f }
        },
        {
            DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -3.0f, 0.0f, 0.0f }),
            DLEngine::Math::Vec3{ 0.0f, 0.0f, 1.0f },
            DLEngine::Math::Vec3{ 1.0f, 1.0f, 0.0f }
        },
    };
    DLEngine::Renderer::SubmitToHologramGroup(cube, hologramGroupInstances);

}

void WorldLayer::OnDetach()
{
    
}

void WorldLayer::OnUpdate(DeltaTime dt)
{
    m_CameraController.OnUpdate(dt);

    DLEngine::Renderer::BeginScene(m_CameraController.GetCamera());

    DLEngine::Renderer::EndScene();
}

void WorldLayer::OnEvent(DLEngine::Event& e)
{
    m_CameraController.OnEvent(e);
}
