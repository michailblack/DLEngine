#include "dlpch.h"
#include "HologramGroup.h"

#include "DLEngine/Math/Distance.h"
#include "DLEngine/Math/Intersections.h"

namespace DLEngine
{
    HologramGroup::HologramGroup()
    {
        const auto& device{ D3D::GetDevice5() };

        ShaderSpecification shaderSpec;
        shaderSpec.Name = "Hologram.vs";
        shaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\Hologram.vs.hlsl";
        m_VertexShader = CreateRef<VertexShader>(shaderSpec);

        shaderSpec.Name = "Hologram.ps";
        shaderSpec.Path = L"..\\DLEngine\\src\\DLEngine\\Shaders\\Hologram.ps.hlsl";
        m_PixelShader = CreateRef<PixelShader>(shaderSpec);

        m_InputLayout = CreateRef<InputLayout>();

        static const BufferLayout instanceBufferLayout{
            { "TRANSFORM"  , BufferLayout::ShaderDataType::Mat4   },
            { "BASE_COLOR" , BufferLayout::ShaderDataType::Float3 },
            { "ADD_COLOR"  , BufferLayout::ShaderDataType::Float3 }
        };
        m_InputLayout->AppendVertexBuffer(instanceBufferLayout, D3D11_INPUT_PER_INSTANCE_DATA);

        m_InstanceBuffer = CreateRef<PerInstanceBuffer<HologramGroupInstance>>(instanceBufferLayout);

        const auto vertexBufferLayout{ Model::GetCommonVertexBufferLayout() };
        m_InputLayout->AppendVertexBuffer(vertexBufferLayout, D3D11_INPUT_PER_VERTEX_DATA);
        m_InputLayout->Construct(m_VertexShader);

        D3D11_RASTERIZER_DESC2 rasterizerDesc{};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_NONE;
        rasterizerDesc.FrontCounterClockwise = true;
        rasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = true;
        rasterizerDesc.ScissorEnable = false;
        rasterizerDesc.MultisampleEnable = false;
        rasterizerDesc.AntialiasedLineEnable = false;
        rasterizerDesc.ForcedSampleCount = 0u;
        rasterizerDesc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        DL_THROW_IF_HR(device->CreateRasterizerState2(&rasterizerDesc, &m_RasterizerState));

        m_PerDrawConstantBuffer = CreateRef<ConstantBuffer<PerDraw>>();
    }

    void HologramGroup::UpdateAndSetPerDrawBuffer(uint32_t modelIndex, uint32_t meshIndex, uint32_t instanceIndex) const
    {
        const auto& perModel{ m_Models[modelIndex] };
        const auto& srcMesh{ perModel.Model->GetMesh(meshIndex) };

        PerDraw perDraw{};
        perDraw.MeshToModel = srcMesh.GetInstance(instanceIndex);
        perDraw.ModelToMesh = srcMesh.GetInvInstance(instanceIndex);

        m_PerDrawConstantBuffer->Set(perDraw);

        m_PerDrawConstantBuffer->BindVS(2u);
    }

    bool HologramGroup::IntersectsInstance(const Math::Ray& ray, IShaderGroup::IntersectInfo& outIntersectInfo) const
    {
        bool intersects{ false };
        const auto& perModel{ m_Models[outIntersectInfo.ModelIndex] };
        const auto& perMesh{ perModel.PerMesh[outIntersectInfo.MeshIndex] };
        const auto& instance{ std::any_cast<HologramGroupInstance>(perMesh.Instances[outIntersectInfo.InstanceIndex]) };

        const Math::Mat4x4& modelToWorld{ instance.Transform };
        const Math::Mat4x4 worldToModel{ Math::Mat4x4::Inverse(modelToWorld) };

        const Math::Ray modelSpaceRay{
            Math::Vec4{ Math::Vec4{ ray.Origin, 1.0f } *worldToModel }.xyz(),
            Math::Normalize(Math::Vec4{ Math::Vec4{ ray.Direction, 0.0f } *worldToModel }.xyz())
        };

        Model::IntersectInfo modelSpaceIntersectInfo{ outIntersectInfo.ModelIntersectInfo };
        if (modelSpaceIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.T != Math::Infinity())
        {
            modelSpaceIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint = Math::Vec4{
                Math::Vec4{
                    outIntersectInfo.ModelIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint, 1.0f
            } *worldToModel }.xyz();
            modelSpaceIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.T = Math::Length(
                modelSpaceIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint - modelSpaceRay.Origin
            );
        }

        if (perModel.Model->Intersects(modelSpaceRay, modelSpaceIntersectInfo))
        {
            const Math::Vec3 worldSpaceIntersectionPoint{ Math::Vec4{
                Math::Vec4{ modelSpaceIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint, 1.0f }
                *modelToWorld
            }.xyz() };
            const float distance{ Math::Length(worldSpaceIntersectionPoint - ray.Origin) };

            if (distance < outIntersectInfo.ModelIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.T)
            {
                outIntersectInfo.ModelIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint = worldSpaceIntersectionPoint;
                outIntersectInfo.ModelIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.Normal = Math::Normalize(
                    Math::Vec4{ Math::Vec4{
                        modelSpaceIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.Normal, 0.0f
                    } *modelToWorld }.xyz());
                outIntersectInfo.ModelIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.T = distance;

                intersects = true;
            }
        }
        return intersects;
    }

    Ref<MeshDragger> HologramGroup::CreateMeshDragger(const Math::Ray& ray, const Math::Vec3& cameraForward, const IShaderGroup::IntersectInfo& intersectInfo)
    {
        const Math::Plane nearPlane{
            .Origin = ray.Origin,
            .Normal = cameraForward
        };

        auto& perModel{ m_Models[intersectInfo.ModelIndex] };
        auto& perMesh{ perModel.PerMesh[intersectInfo.MeshIndex] };
        auto& instance{ std::any_cast<HologramGroupInstance&>(perMesh.Instances[intersectInfo.InstanceIndex]) };

        return CreateRef<HologramGroupDragger>(
            instance,
            intersectInfo.ModelIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint,
            Math::Distance(intersectInfo.ModelIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo.IntersectionPoint, nearPlane)
        );
    }

    void HologramGroupDragger::Drag(const Math::Plane& nearPlane, const Math::Ray& endRay)
    {
        const Math::Plane draggingPlane{
            .Origin = nearPlane.Origin + nearPlane.Normal * m_DistanceToDraggingPlane,
            .Normal = nearPlane.Normal
        };

        Math::IntersectInfo intersectInfo{};
        DL_ASSERT_EXPR(Math::Intersects(endRay, draggingPlane, intersectInfo));

        const Math::Vec3 translation = intersectInfo.IntersectionPoint - m_StartDraggingPoint;

        m_Instance.Transform = m_Instance.Transform * Math::Mat4x4::Translate(translation);

        m_StartDraggingPoint = intersectInfo.IntersectionPoint;
    }
}
