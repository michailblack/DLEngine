#include "WorldLayer.h"

#include "DLEngine/DirectX/D3D.h"

#include "DLEngine/Mesh/MeshSystem.h"
#include "DLEngine/Mesh/ModelManager.h"
#include "DLEngine/DirectX/DXStates.h"

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

    struct NullMaterial
    {
        bool operator==(const NullMaterial&) const { return true; }
    };

    struct NormalVisGroupInstance
    {
        DLEngine::Math::Mat4x4 ModelToWorld;
    };

    DLEngine::ShadingGroupDesc normalVisGroupDesc{};
    normalVisGroupDesc.InstanceBufferLayout = DLEngine::BufferLayout{
        { "TRANSFORM", DLEngine::BufferLayout::ShaderDataType::Mat4 }
    };
    normalVisGroupDesc.VertexShaderSpec.Name = "NormalVis.vs";
    normalVisGroupDesc.VertexShaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\NormalVis.vs.hlsl";

    normalVisGroupDesc.PixelShaderSpec.Name = "NormalVis.ps";
    normalVisGroupDesc.PixelShaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\NormalVis.ps.hlsl";

    normalVisGroupDesc.DepthStencilState = DLEngine::DXStates::GetDepthStencilState(DLEngine::DepthStencilStates::Default);
    normalVisGroupDesc.RasterizerState = DLEngine::DXStates::GetRasterizerState(DLEngine::RasterizerStates::Default);

    DLEngine::MeshSystem::Get().CreateShadingGroup<NullMaterial, NormalVisGroupInstance>(normalVisGroupDesc);

    std::vector<NullMaterial> nullMaterials{};
    nullMaterials.resize(cube->GetMeshesCount());

    NormalVisGroupInstance normalVisGroupInstance{};
    normalVisGroupInstance.ModelToWorld = DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 2.0f, 0.0f, 3.0f });
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, normalVisGroupInstance);

    normalVisGroupInstance.ModelToWorld = DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -2.0f, 0.0f, 3.0f });
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, normalVisGroupInstance);

    struct HologramGroupInstance
    {
        DLEngine::Math::Mat4x4 ModelToWorld;
        DLEngine::Math::Vec3 BaseColor;
        DLEngine::Math::Vec3 AdditionalColor;
    };

    DLEngine::ShadingGroupDesc hologramGroupDesc{};
    hologramGroupDesc.InstanceBufferLayout = DLEngine::BufferLayout{
        { "TRANSFORM" , DLEngine::BufferLayout::ShaderDataType::Mat4   },
        { "BASE_COLOR", DLEngine::BufferLayout::ShaderDataType::Float3 },
        { "ADD_COLOR" , DLEngine::BufferLayout::ShaderDataType::Float3 }
    };
    
    hologramGroupDesc.VertexShaderSpec.Name = "Hologram.vs";
    hologramGroupDesc.VertexShaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\Hologram.vs.hlsl";

    hologramGroupDesc.PixelShaderSpec.Name = "Hologram.ps";
    hologramGroupDesc.PixelShaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\Hologram.ps.hlsl";

    hologramGroupDesc.DepthStencilState = DLEngine::DXStates::GetDepthStencilState(DLEngine::DepthStencilStates::Default);
    hologramGroupDesc.RasterizerState = DLEngine::DXStates::GetRasterizerState(DLEngine::RasterizerStates::Default);

    DLEngine::MeshSystem::Get().CreateShadingGroup<NullMaterial, HologramGroupInstance>(hologramGroupDesc);

    HologramGroupInstance hologramGroupInstance{};
    hologramGroupInstance.ModelToWorld = DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 0.0f, 5.0f, 8.0f });
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 1.0f, 0.0f, 1.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 0.0f, 1.0f, 1.0f };
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, hologramGroupInstance);

    hologramGroupInstance.ModelToWorld = DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -3.0f, 0.0f, 0.0f });
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 0.0f, 0.0f, 1.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 1.0f, 1.0f, 0.0f };
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, hologramGroupInstance);
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
