#include "dlpch.h"
#include "ReflectionCapture.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/RenderCommand.h"

#include "DLEngine/Renderer/TextureManager.h"

#include "DLEngine/Systems/Mesh/Model.h"
#include "DLEngine/Systems/Mesh/ModelManager.h"

#include "DLEngine/Math/Math.h"

namespace DLEngine
{
    namespace
    {
        struct ReflectionCaptureCB
        {
            Math::Mat4x4 View;
            Math::Mat4x4 Projection;
            uint32_t EnvironmentMapSize;
            float Roughness;
            float _padding[2]{};
        };
    }

    void ReflectionCapture::Create(uint32_t numHemispherePoints)
    {
        const std::vector<Math::Vec3> hemispherePoints{ Math::GenerateFibonacciHemispherePoints(numHemispherePoints) };

        m_HemispherePointsSB.Create(sizeof(Math::Vec3), numHemispherePoints, hemispherePoints.data());

        m_ConstantBuffer.Create(sizeof(ReflectionCaptureCB));

        m_TextureSize = numHemispherePoints / 4u;

        D3D11_TEXTURE2D_DESC1 textureDesc{};
        textureDesc.Width = m_TextureSize;
        textureDesc.Height = m_TextureSize;
        textureDesc.MipLevels = 1u;
        textureDesc.ArraySize = 6u;
        textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        textureDesc.SampleDesc.Count = 1u;
        textureDesc.SampleDesc.Quality = 0u;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0u;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        textureDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

        m_DiffuseIrradianceMap.Create(textureDesc);
        m_DiffuseIrradianceMap.SetDebugName("DiffuseIrradianceMap");

        m_NumSpecularMipLevels = static_cast<uint32_t>(Math::Log2(static_cast<float>(m_TextureSize)));
        textureDesc.MipLevels = m_NumSpecularMipLevels;

        m_SpecularIrradianceMap.Create(textureDesc);
        m_SpecularIrradianceMap.SetDebugName("SpecularIrradianceMap");

        textureDesc.MipLevels = 1u;
        textureDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
        textureDesc.ArraySize = 1u;
        textureDesc.MiscFlags = 0u;

        m_SpecularFactorMap.Create(textureDesc);
        m_SpecularFactorMap.SetDebugName("SpecularFactorMap");

        m_PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        ShaderSpecification shaderSpec{};

        shaderSpec.Path = Filesystem::GetShaderDir() + L"ReflectionCapture.hlsl";
        shaderSpec.EntryPoint = "mainVS";
        m_PipelineState.VS.Create(shaderSpec);

        shaderSpec.EntryPoint = "mainPSDiffuseIrradiance";
        m_DiffuseIrradiancePS.Create(shaderSpec);

        shaderSpec.EntryPoint = "mainPSSpecularIrradiance";
        m_SpecularIrradiancePS.Create(shaderSpec);

        shaderSpec.EntryPoint = "mainPSSpecularFactor";
        m_SpecularFactorPS.Create(shaderSpec);

        const auto vertexLayout{ Model::GetCommonVertexBufferLayout() };
        m_PipelineState.Layout.AppendVertexBuffer(vertexLayout, D3D11_INPUT_PER_VERTEX_DATA);
        m_PipelineState.Layout.Create(m_PipelineState.VS);

        m_PipelineState.Rasterizer = D3DStates::GetRasterizerState(RasterizerStates::CULL_NONE);
        m_PipelineState.DepthStencil = D3DStates::GetDepthStencilState(DepthStencilStates::DEPTH_DISABLED);
    }

    void ReflectionCapture::Build(const Texture2D& skybox)
    {
        const auto cube{ ModelManager::Load(Filesystem::GetModelDir() + L"cube\\cube.obj") };

        RenderCommand::SetVertexBuffers(0u, { cube->GetVertexBuffer() });
        RenderCommand::SetIndexBuffer(cube->GetIndexBuffer());
        RenderCommand::SetPipelineState(m_PipelineState);

        RenderCommand::SetConstantBuffers(3u, ShaderStage::Vertex, { m_ConstantBuffer });
        RenderCommand::SetConstantBuffers(3u, ShaderStage::Pixel, { m_ConstantBuffer });

        RenderCommand::SetShaderResources(5u, ShaderStage::Pixel, { m_HemispherePointsSB.GetSRV(), skybox.GetSRV() });

        RenderCommand::SetSamplers(0u, ShaderStage::Pixel, { D3DStates::GetSamplerState(SamplerStates::ANISOTROPIC_8_CLAMP) });

        static const Math::Vec3 pos{ 0.0f, 0.0f, 0.0f };
        static const std::array<Math::Mat4x4, 6u> views{
            // +X
            Math::Mat4x4::View(Math::Vec3{0.0, 0.0, -1.0}, Math::Vec3{0.0, 1.0, 0.0}, Math::Vec3{1.0, 0.0, 0.0}, pos),

            // -X
            Math::Mat4x4::View(Math::Vec3{0.0, 0.0, 1.0}, Math::Vec3{0.0, 1.0, 0.0}, Math::Vec3{-1.0, 0.0, 0.0}, pos),

            // +Y
            Math::Mat4x4::View(Math::Vec3{1.0, 0.0, 0.0}, Math::Vec3{0.0, 0.0, -1.0}, Math::Vec3{0.0, 1.0, 0.0}, pos),

            // -Y
            Math::Mat4x4::View(Math::Vec3{1.0, 0.0, 0.0}, Math::Vec3{0.0, 0.0, 1.0}, Math::Vec3{0.0, -1.0, 0.0}, pos),

            // +Z
            Math::Mat4x4::View(Math::Vec3{1.0, 0.0, 0.0}, Math::Vec3{0.0, 1.0, 0.0}, Math::Vec3{0.0, 0.0, 1.0}, pos),

            // -Z
            Math::Mat4x4::View(Math::Vec3{-1.0, 0.0, 0.0}, Math::Vec3{0.0, 1.0, 0.0}, Math::Vec3{0.0, 0.0, -1.0}, pos) };

        static const Math::Mat4x4 proj{ Math::Mat4x4::Perspective(Math::ToRadians(90.0f), 1.0f, 0.001f, 100.0f) };

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        ReflectionCaptureCB cb{};
        cb.Projection = proj;
        cb.EnvironmentMapSize = skybox.GetDesc().Width;

        for (uint32_t i{ 0u }; i < 6u; ++i)
        {
            cb.View = views[i];
            m_ConstantBuffer.Set(&cb);

            viewport.Width = static_cast<float>(m_TextureSize);
            viewport.Height = static_cast<float>(m_TextureSize);

            RenderCommand::SetViewports({ viewport });

            RenderCommand::SetRenderTargets(
                {
                    m_DiffuseIrradianceMap.GetRTV(0u, i),
                });

            RenderCommand::SetPixelShader(m_DiffuseIrradiancePS);
            RenderCommand::DrawIndexed(cube->GetMeshRange(0u).IndexCount);

            const float roughnessStep{ 1.0f / static_cast<float>(m_NumSpecularMipLevels) };
            for (uint32_t mip{ 0u }; mip < m_NumSpecularMipLevels; ++mip)
            {
                cb.Roughness = static_cast<float>(mip) * roughnessStep;
                m_ConstantBuffer.Set(&cb);

                const uint32_t mipSize{ m_TextureSize >> mip };

                viewport.Width = static_cast<float>(mipSize);
                viewport.Height = static_cast<float>(mipSize);

                RenderCommand::SetViewports({ viewport });

                RenderCommand::SetRenderTargets(
                    {
                        m_SpecularIrradianceMap.GetRTV(mip, i),
                    });

                RenderCommand::SetPixelShader(m_SpecularIrradiancePS);
                RenderCommand::DrawIndexed(cube->GetMeshRange(0u).IndexCount);
            }
        }

        viewport.Width = static_cast<float>(m_TextureSize);
        viewport.Height = static_cast<float>(m_TextureSize);

        RenderCommand::SetViewports({ viewport });

        cb.View = views[4];
        m_ConstantBuffer.Set(&cb);

        RenderCommand::SetRenderTargets(
            {
                m_SpecularFactorMap.GetRTV(),
            });
        RenderCommand::SetPixelShader(m_SpecularFactorPS);
        RenderCommand::DrawIndexed(cube->GetMeshRange(0u).IndexCount);

        RenderCommand::SetRenderTargets();

        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(Application::Get().GetWindow()->GetWidth());
        viewport.Height = static_cast<float>(Application::Get().GetWindow()->GetHeight());
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        RenderCommand::SetViewports({ viewport });

        TextureManager::SaveToDDS(m_DiffuseIrradianceMap, L"DiffuseIrradianceIBL");
        TextureManager::SaveToDDS(m_SpecularIrradianceMap, L"SpecularIrradianceIBL");
        TextureManager::SaveToDDS(m_SpecularFactorMap, L"SpecularFactorIBL");
    }
}
