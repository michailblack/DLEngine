#include "WorldLayer.h"

#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/DXStates.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"
#include "DLEngine/Systems/Mesh/ModelManager.h"

#include "DLEngine/Systems/Renderer/Renderer.h"

#include "DLEngine/Systems/Transform/TransformSystem.h"

struct HologramGroupInstance
{
    DLEngine::Math::Vec3 BaseColor;
    DLEngine::Math::Vec3 AdditionalColor;
};

WorldLayer::WorldLayer()
    : m_CameraController(DLEngine::Camera { DLEngine::Math::ToRadians(45.0f), 800.0f / 600.0f, 0.001f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    const auto cube{ DLEngine::ModelManager::Load(DLEngine::Filesystem::GetModelDir() + "cube\\cube.obj") };
    const auto samurai{ DLEngine::ModelManager::Load(DLEngine::Filesystem::GetModelDir() + "samurai\\samurai.fbx") };

    std::vector<DLEngine::NullMaterial> nullMaterials{};
    uint32_t transformIndex{ 0u };

    InitHologramGroup();

    nullMaterials.resize(cube->GetMeshesCount());
    HologramGroupInstance hologramGroupInstance{};
    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 0.0f, 5.0f, 8.0f })
    );
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 1.0f, 0.0f, 1.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 0.0f, 1.0f, 1.0f };
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, hologramGroupInstance, transformIndex);

    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Rotate(DLEngine::Math::Normalize(DLEngine::Math::Vec3{ 1.0f, 1.0f, 1.0f }), DLEngine::Math::ToRadians(45.0f)) *
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -3.0f, 0.0f, 0.0f })
    );
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 0.0f, 0.0f, 1.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 1.0f, 1.0f, 0.0f };
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, hologramGroupInstance, transformIndex);

    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Scale(DLEngine::Math::Vec3{ 0.5f, 1.0f, 1.5f }) *
        DLEngine::Math::Mat4x4::Rotate(DLEngine::Math::Normalize(DLEngine::Math::Vec3{ 1.0f, 1.0f, 1.0f }), DLEngine::Math::ToRadians(45.0f)) *
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 0.0f, -5.0f, 5.0f })
    );
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 1.0f, 0.0f, 0.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 0.0f, 1.0f, 1.0f };
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, hologramGroupInstance, transformIndex);

    nullMaterials.resize(samurai->GetMeshesCount());
    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -3.0f, -2.0f, 4.0f })
    );
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 1.0f, 1.0f, 0.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 0.0f, 1.0f, 0.0f };
    DLEngine::MeshSystem::Get().Add<>(samurai, nullMaterials, hologramGroupInstance, transformIndex);

    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Rotate(DLEngine::Math::Normalize(DLEngine::Math::Vec3{ 1.0f, 1.0f, 1.0f }), DLEngine::Math::ToRadians(-30.0f)) *
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 3.0f, 2.0f, 4.0f })
    );
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 0.0f, 1.0f, 0.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 0.0f, 1.0f, 1.0f };
    DLEngine::MeshSystem::Get().Add<>(samurai, nullMaterials, hologramGroupInstance, transformIndex);

    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Scale(DLEngine::Math::Vec3{ 1.5f, 0.5f, 0.75f }) *
        DLEngine::Math::Mat4x4::Rotate(DLEngine::Math::Normalize(DLEngine::Math::Vec3{ 1.0f, 1.0f, 1.0f }), DLEngine::Math::ToRadians(-30.0f)) *
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 0.0f, 3.0f, 5.0f })
    );
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 0.1f, 0.3f, 0.7f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 1.0f, 1.0f, 1.0f };
    DLEngine::MeshSystem::Get().Add<>(samurai, nullMaterials, hologramGroupInstance, transformIndex);
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

void WorldLayer::InitHologramGroup() const
{
    DLEngine::ShadingGroupDesc hologramGroupDesc{};

    hologramGroupDesc.Name = "Hologram";

    hologramGroupDesc.PipelineDesc.Topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

    hologramGroupDesc.InstanceBufferLayout = DLEngine::BufferLayout{
        { "TRANSFORM" , DLEngine::BufferLayout::ShaderDataType::Mat4   },
        { "BASE_COLOR", DLEngine::BufferLayout::ShaderDataType::Float3 },
        { "ADD_COLOR" , DLEngine::BufferLayout::ShaderDataType::Float3 }
    };

    DLEngine::ShaderSpecification shaderSpec{};

    shaderSpec.Name = "Hologram.vs";
    shaderSpec.Path = DLEngine::Filesystem::GetShaderDir() + "Hologram.vs.hlsl";
    DLEngine::VertexShader vs{};
    vs.Create(shaderSpec);

    hologramGroupDesc.PipelineDesc.VS = vs;

    shaderSpec.Name = "Hologram.ps";
    shaderSpec.Path = DLEngine::Filesystem::GetShaderDir() + "Hologram.ps.hlsl";
    DLEngine::PixelShader ps{};
    ps.Create(shaderSpec);

    hologramGroupDesc.PipelineDesc.PS = ps;

    shaderSpec.Name = "Hologram.hs";
    shaderSpec.Path = DLEngine::Filesystem::GetShaderDir() + "Hologram.hs.hlsl";
    DLEngine::HullShader hs{};
    hs.Create(shaderSpec);

    hologramGroupDesc.PipelineDesc.HS = hs;

    shaderSpec.Name = "Hologram.ds";
    shaderSpec.Path = DLEngine::Filesystem::GetShaderDir() + "Hologram.ds.hlsl";
    DLEngine::DomainShader ds{};
    ds.Create(shaderSpec);
    
    hologramGroupDesc.PipelineDesc.DS = ds;
    
    shaderSpec.Name = "Hologram.gs";
    shaderSpec.Path = DLEngine::Filesystem::GetShaderDir() + "Hologram.gs.hlsl";
    DLEngine::GeometryShader gs{};
    gs.Create(shaderSpec);

    hologramGroupDesc.PipelineDesc.GS = gs;

    hologramGroupDesc.PipelineDesc.DepthStencilState = DLEngine::DXStates::GetDepthStencilState(DLEngine::DepthStencilStates::Default);
    hologramGroupDesc.PipelineDesc.RasterizerState = DLEngine::DXStates::GetRasterizerState(DLEngine::RasterizerStates::Default);

    DLEngine::MeshSystem::Get().CreateShadingGroup<DLEngine::NullMaterial, HologramGroupInstance>(hologramGroupDesc);
}
