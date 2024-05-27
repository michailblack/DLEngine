#include "WorldLayer.h"

#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/DXStates.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"
#include "DLEngine/Systems/Mesh/ModelManager.h"

#include "DLEngine/Systems/Renderer/Renderer.h"

#include "DLEngine/Systems/Transform/TransformSystem.h"

WorldLayer::WorldLayer()
    : m_CameraController(DLEngine::Camera { DLEngine::Math::ToRadians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f })
{
}

WorldLayer::~WorldLayer()
{
}

void WorldLayer::OnAttach()
{
    const auto cube{ DLEngine::ModelManager::Load(DLEngine::Filesystem::GetModelDir() + "cube\\cube.obj") };
    const auto samurai{ DLEngine::ModelManager::Load(DLEngine::Filesystem::GetModelDir() + "samurai\\samurai.fbx") };

    struct NullMaterial
    {
        bool operator==(const NullMaterial&) const { return true; }
    };

    std::vector<NullMaterial> nullMaterials{};
    nullMaterials.resize(cube->GetMeshesCount());
    uint32_t transformIndex{ 0u };

    struct NormalVisGroupInstance
    {
        /// Empty struct has a size of 1 byte, which ShadingGroup can't handle
        /// right now when building its instance buffer, so we need to add an empty data flag here,
        /// in the shader, and in the buffer layout as well
        float _emptyInstance;
    };

    DLEngine::ShadingGroupDesc normalVisGroupDesc{};
    normalVisGroupDesc.InstanceBufferLayout = DLEngine::BufferLayout{
        { "TRANSFORM" , DLEngine::BufferLayout::ShaderDataType::Mat4  },
        { "_empty"    , DLEngine::BufferLayout::ShaderDataType::Float }
    };
    normalVisGroupDesc.VertexShaderSpec.Name = "NormalVis.vs";
    normalVisGroupDesc.VertexShaderSpec.Path = DLEngine::Filesystem::GetShaderDir() + "NormalVis.vs.hlsl";

    normalVisGroupDesc.PixelShaderSpec.Name = "NormalVis.ps";
    normalVisGroupDesc.PixelShaderSpec.Path = DLEngine::Filesystem::GetShaderDir() + "NormalVis.ps.hlsl";

    normalVisGroupDesc.DepthStencilState = DLEngine::DXStates::GetDepthStencilState(DLEngine::DepthStencilStates::Default);
    normalVisGroupDesc.RasterizerState = DLEngine::DXStates::GetRasterizerState(DLEngine::RasterizerStates::Default);

    DLEngine::MeshSystem::Get().CreateShadingGroup<NullMaterial, NormalVisGroupInstance>(normalVisGroupDesc);

    NormalVisGroupInstance normalVisGroupInstance{};
    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 2.0f, 0.0f, 3.0f })
    );
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, normalVisGroupInstance, transformIndex);

    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -2.0f, 0.0f, 3.0f })
    );
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, normalVisGroupInstance, transformIndex);

    nullMaterials.resize(samurai->GetMeshesCount());
    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 0.0f, 0.0f, 6.0f })
    );
    DLEngine::MeshSystem::Get().Add<>(samurai, nullMaterials, normalVisGroupInstance, transformIndex);

    struct HologramGroupInstance
    {
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
    hologramGroupDesc.VertexShaderSpec.Path = DLEngine::Filesystem::GetShaderDir() + "Hologram.vs.hlsl";

    hologramGroupDesc.PixelShaderSpec.Name = "Hologram.ps";
    hologramGroupDesc.PixelShaderSpec.Path = DLEngine::Filesystem::GetShaderDir() + "Hologram.ps.hlsl";

    hologramGroupDesc.DepthStencilState = DLEngine::DXStates::GetDepthStencilState(DLEngine::DepthStencilStates::Default);
    hologramGroupDesc.RasterizerState = DLEngine::DXStates::GetRasterizerState(DLEngine::RasterizerStates::Default);

    DLEngine::MeshSystem::Get().CreateShadingGroup<NullMaterial, HologramGroupInstance>(hologramGroupDesc);

    nullMaterials.resize(cube->GetMeshesCount());
    HologramGroupInstance hologramGroupInstance{};
    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ 0.0f, 5.0f, 8.0f })
    );
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 1.0f, 0.0f, 1.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 0.0f, 1.0f, 1.0f };
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, hologramGroupInstance, transformIndex);

    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -3.0f, 0.0f, 0.0f })
    );
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 0.0f, 0.0f, 1.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 1.0f, 1.0f, 0.0f };
    DLEngine::MeshSystem::Get().Add<>(cube, nullMaterials, hologramGroupInstance, transformIndex);

    nullMaterials.resize(samurai->GetMeshesCount());
    transformIndex = DLEngine::TransformSystem::AddTransform(
        DLEngine::Math::Mat4x4::Translate(DLEngine::Math::Vec3{ -6.0f, 4.0f, 2.0f })
    );
    hologramGroupInstance.BaseColor = DLEngine::Math::Vec3{ 1.0f, 1.0f, 0.0f };
    hologramGroupInstance.AdditionalColor = DLEngine::Math::Vec3{ 0.0f, 1.0f, 0.0f };
    DLEngine::MeshSystem::Get().Add<>(samurai, nullMaterials, hologramGroupInstance, transformIndex);
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
