#include "dlpch.h"
#include "D3D11Renderer.h"

#include "DLEngine/Core/Application.h"

#include "DLEngine/DirectX/D3D11Common.h"
#include "DLEngine/DirectX/D3D11ConstantBuffer.h"
#include "DLEngine/DirectX/D3D11Context.h"
#include "DLEngine/DirectX/D3D11Framebuffer.h"
#include "DLEngine/DirectX/D3D11IndexBuffer.h"
#include "DLEngine/DirectX/D3D11Material.h"
#include "DLEngine/DirectX/D3D11Pipeline.h"
#include "DLEngine/DirectX/D3D11Shader.h"
#include "DLEngine/DirectX/D3D11Texture.h"
#include "DLEngine/DirectX/D3D11VertexBuffer.h"
#include "DLEngine/DirectX/D3D11StructuredBuffer.h"

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            D3D11_TEXTURE_ADDRESS_MODE D3D11AddressFromTextureAddress(TextureAddress address);
            D3D11_FILTER D3D11FilterFromTextureFilter(TextureFilter filter);
        }
    }

    namespace
    {
        struct D3D11RendererData
        {
            std::unordered_map<SamplerSpecification, Microsoft::WRL::ComPtr<ID3D11SamplerState>, SamplerSpecificationHash> SamplersCache;
            std::unordered_map<bool, Microsoft::WRL::ComPtr<ID3D11RasterizerState2>> RasterizerStatesCache;
            std::unordered_map<DepthStencilSpecification, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>, DepthStencilSpecificationHash> DepthStencilStatesCache;
        };

        D3D11RendererData* s_Data{ nullptr };
    }

    void D3D11Renderer::Init()
    {
        s_Data = new D3D11RendererData;
    }

    void D3D11Renderer::Shutdown()
    {
        delete s_Data;
    }

    void D3D11Renderer::BeginFrame()
    {
        const auto& d3d11deviceContext{ D3D11Context::Get()->GetDeviceContext4() };

        d3d11deviceContext->ClearState();

        const std::vector<SamplerSpecification> globalSamplers{
            SamplerSpecification{ TextureAddress::Wrap, TextureFilter::Anisotropic8, CompareOperator::Never},
            SamplerSpecification{ TextureAddress::Wrap, TextureFilter::Nearest, CompareOperator::Never },
            SamplerSpecification{ TextureAddress::Clamp, TextureFilter::Nearest, CompareOperator::Never },
            SamplerSpecification{ TextureAddress::Wrap, TextureFilter::Trilinear, CompareOperator::Never },
            SamplerSpecification{ TextureAddress::Clamp, TextureFilter::Trilinear, CompareOperator::Never },
            SamplerSpecification{ TextureAddress::Wrap, TextureFilter::Anisotropic8, CompareOperator::Never },
            SamplerSpecification{ TextureAddress::Clamp, TextureFilter::Anisotropic8, CompareOperator::Never }
        };

        SetSamplerStates(0u, DL_PIXEL_SHADER_BIT, globalSamplers);
    }

    void D3D11Renderer::EndFrame()
    {
    }

    Ref<Texture2D> D3D11Renderer::GetBackBufferTexture()
    {
        const auto& d3d11BackBufferTexture{ Application::Get().GetWindow().GetSwapChain()->GetD3D11BackBuffer() };

        D3D11_TEXTURE2D_DESC1 backBufferDesc{};
        d3d11BackBufferTexture->GetDesc1(&backBufferDesc);

        TextureSpecification textureSpec{};
        textureSpec.DebugName = "Standard back buffer attachment";
        textureSpec.Format = Utils::TextureFormatFromDXGIFormat(backBufferDesc.Format);
        textureSpec.Usage = TextureUsage::Attachment;
        textureSpec.Width = backBufferDesc.Width;
        textureSpec.Height = backBufferDesc.Height;

        return AsRef<Texture2D>(CreateRef<D3D11Texture2D>(d3d11BackBufferTexture, textureSpec));
    }

    void D3D11Renderer::SetConstantBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<ConstantBuffer>>& constantBuffers) noexcept
    {
        const auto& d3d11deviceContext{ D3D11Context::Get()->GetDeviceContext4() };

        std::vector<ID3D11Buffer*> d3d11ConstantBuffers;
        d3d11ConstantBuffers.reserve(constantBuffers.size());

        for (const auto& constantBuffer : constantBuffers)
            d3d11ConstantBuffers.push_back(AsRef<D3D11ConstantBuffer>(constantBuffer)->GetD3D11ConstantBuffer().Get());

        if (shaderStageFlags & ShaderStage::DL_VERTEX_SHADER_BIT)
            d3d11deviceContext->VSSetConstantBuffers(startSlot, static_cast<UINT>(d3d11ConstantBuffers.size()), d3d11ConstantBuffers.data());

        if (shaderStageFlags & ShaderStage::DL_PIXEL_SHADER_BIT)
            d3d11deviceContext->PSSetConstantBuffers(startSlot, static_cast<UINT>(d3d11ConstantBuffers.size()), d3d11ConstantBuffers.data());

        if (shaderStageFlags & ShaderStage::DL_HULL_SHADER_BIT)
            d3d11deviceContext->HSSetConstantBuffers(startSlot, static_cast<UINT>(d3d11ConstantBuffers.size()), d3d11ConstantBuffers.data());

        if (shaderStageFlags & ShaderStage::DL_DOMAIN_SHADER_BIT)
            d3d11deviceContext->DSSetConstantBuffers(startSlot, static_cast<UINT>(d3d11ConstantBuffers.size()), d3d11ConstantBuffers.data());

        if (shaderStageFlags & ShaderStage::DL_GEOMETRY_SHADER_BIT)
            d3d11deviceContext->GSSetConstantBuffers(startSlot, static_cast<UINT>(d3d11ConstantBuffers.size()), d3d11ConstantBuffers.data());

        if (shaderStageFlags & ShaderStage::DL_COMPUTE_SHADER_BIT)
            d3d11deviceContext->CSSetConstantBuffers(startSlot, static_cast<UINT>(d3d11ConstantBuffers.size()), d3d11ConstantBuffers.data());
    }

    void D3D11Renderer::SetTexture2Ds(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<Texture2D>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept
    {
        DL_ASSERT(textures.size() == viewSpecifications.size(), "Textures count and view specifications count does not match");

        std::vector<ID3D11ShaderResourceView*> d3d11ShaderResourceViews;
        d3d11ShaderResourceViews.reserve(textures.size());

        for (uint32_t i{ 0u }; i < textures.size(); ++i)
            d3d11ShaderResourceViews.push_back(AsRef<D3D11Texture2D>(textures[i])->GetD3D11ShaderResourceView(viewSpecifications[i]).Get());

        SetShaderResourceViews(startSlot, shaderStageFlags, d3d11ShaderResourceViews);
    }

    void D3D11Renderer::SetTextureCubes(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<TextureCube>>& textures, const std::vector<TextureViewSpecification>& viewSpecifications) noexcept
    {
        DL_ASSERT(textures.size() == viewSpecifications.size(), "Textures count and view specifications count does not match");

        std::vector<ID3D11ShaderResourceView*> d3d11ShaderResourceViews;
        d3d11ShaderResourceViews.reserve(textures.size());

        for (uint32_t i{ 0u }; i < textures.size(); ++i)
            d3d11ShaderResourceViews.push_back(AsRef<D3D11TextureCube>(textures[i])->GetD3D11ShaderResourceView(viewSpecifications[i]).Get());

        SetShaderResourceViews(startSlot, shaderStageFlags, d3d11ShaderResourceViews);
    }

    void D3D11Renderer::SetStructuredBuffers(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<Ref<StructuredBuffer>>& structuredBuffers, const std::vector<BufferViewSpecification>& viewSpecifications) noexcept
    {
        DL_ASSERT(structuredBuffers.size() == viewSpecifications.size(), "Structured buffers count and view specifications count does not match");

        std::vector<ID3D11ShaderResourceView*> d3d11ShaderResourceViews;
        d3d11ShaderResourceViews.reserve(structuredBuffers.size());

        for (uint32_t i{ 0u }; i < structuredBuffers.size(); ++i)
            d3d11ShaderResourceViews.push_back(AsRef<D3D11StructuredBuffer>(structuredBuffers[i])->GetD3D11ShaderResourceView(viewSpecifications[i]).Get());

        SetShaderResourceViews(startSlot, shaderStageFlags, d3d11ShaderResourceViews);
    }

    void D3D11Renderer::SetSamplerStates(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<SamplerSpecification>& samplerStates) noexcept
    {
        const auto& d3d11deviceContext{ D3D11Context::Get()->GetDeviceContext4() };

        std::vector<ID3D11SamplerState*> d3d11SamplerStates;
        d3d11SamplerStates.reserve(samplerStates.size());

        for (const auto& samplerState : samplerStates)
            d3d11SamplerStates.push_back(GetSamplerState(samplerState).Get());

        if (shaderStageFlags & ShaderStage::DL_VERTEX_SHADER_BIT)
            d3d11deviceContext->VSSetSamplers(startSlot, static_cast<UINT>(d3d11SamplerStates.size()), d3d11SamplerStates.data());

        if (shaderStageFlags & ShaderStage::DL_PIXEL_SHADER_BIT)
            d3d11deviceContext->PSSetSamplers(startSlot, static_cast<UINT>(d3d11SamplerStates.size()), d3d11SamplerStates.data());

        if (shaderStageFlags & ShaderStage::DL_HULL_SHADER_BIT)
            d3d11deviceContext->HSSetSamplers(startSlot, static_cast<UINT>(d3d11SamplerStates.size()), d3d11SamplerStates.data());

        if (shaderStageFlags & ShaderStage::DL_DOMAIN_SHADER_BIT)
            d3d11deviceContext->DSSetSamplers(startSlot, static_cast<UINT>(d3d11SamplerStates.size()), d3d11SamplerStates.data());

        if (shaderStageFlags & ShaderStage::DL_GEOMETRY_SHADER_BIT)
            d3d11deviceContext->GSSetSamplers(startSlot, static_cast<UINT>(d3d11SamplerStates.size()), d3d11SamplerStates.data());

        if (shaderStageFlags & ShaderStage::DL_COMPUTE_SHADER_BIT)
            d3d11deviceContext->CSSetSamplers(startSlot, static_cast<UINT>(d3d11SamplerStates.size()), d3d11SamplerStates.data());
    }

    void D3D11Renderer::SetPipeline(const Ref<Pipeline>& pipeline, bool clearAttachments) noexcept
    {
        const auto& d3d11DeviceContext{ D3D11Context::Get()->GetDeviceContext4() };
        const auto& d3d11Pipeline{ AsRef<D3D11Pipeline>(pipeline) };
        const auto& d3d11Shader{ AsRef<D3D11Shader>(d3d11Pipeline->GetSpecification().Shader) };
        const auto& d3d11Framebuffer{ AsRef<D3D11Framebuffer>(d3d11Pipeline->GetSpecification().TargetFramebuffer) };

        const auto& d3d11FramebufferSpec{ d3d11Framebuffer->GetSpecification() };

        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(d3d11FramebufferSpec.Width);
        viewport.Height = static_cast<float>(d3d11FramebufferSpec.Height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        d3d11DeviceContext->RSSetViewports(1u, &viewport);

        d3d11DeviceContext->IASetPrimitiveTopology(d3d11Pipeline->GetD3D11PrimitiveTopology());
        d3d11DeviceContext->IASetInputLayout(d3d11Shader->GetD3D11InputLayout().Get());
        d3d11DeviceContext->VSSetShader(d3d11Shader->GetD3D11VertexShader().Get(), nullptr, 0u);
        d3d11DeviceContext->PSSetShader(d3d11Shader->GetD3D11PixelShader().Get(), nullptr, 0u);
        d3d11DeviceContext->HSSetShader(d3d11Shader->GetD3D11HullShader().Get(), nullptr, 0u);
        d3d11DeviceContext->DSSetShader(d3d11Shader->GetD3D11DomainShader().Get(), nullptr, 0u);
        d3d11DeviceContext->GSSetShader(d3d11Shader->GetD3D11GeometryShader().Get(), nullptr, 0u);

        d3d11DeviceContext->OMSetDepthStencilState(GetDepthStencilState(d3d11Pipeline->GetSpecification().DepthStencilState).Get(), 0u);
        d3d11DeviceContext->RSSetState(GetRasterizerState(d3d11Pipeline->GetSpecification().BackFaceCulling).Get());

        const uint32_t colorAttachmentCount{ d3d11Framebuffer->GetColorAttachmentCount() };

        DL_ASSERT(colorAttachmentCount <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, "Color attachment count exceeds the maximum number of simultaneous render targets availabe in D3D11");

        std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> renderTargetViews{};
        for (uint32_t i{ 0u }; i < colorAttachmentCount; ++i)
        {
            const auto& viewSpecification{ d3d11Framebuffer->GetColorAttachmentViewSpecification(i) };
            switch (d3d11FramebufferSpec.AttachmentsType)
            {
            case TextureType::Texture2D:
                renderTargetViews[i] = AsRef<D3D11Texture2D>(d3d11Framebuffer->GetColorAttachment(i))->GetD3D11RenderTargetView(viewSpecification).Get();
                break;
            case TextureType::TextureCube:
                renderTargetViews[i] = AsRef<D3D11TextureCube>(d3d11Framebuffer->GetColorAttachment(i))->GetD3D11RenderTargetView(viewSpecification).Get();
                break;
            case TextureType::None:
            default:
                DL_ASSERT(false, "Framebuffer [{0}] has invalid attachments type", d3d11FramebufferSpec.DebugName);
                break;
            }
        }

        ID3D11DepthStencilView* depthStencilView{ nullptr };
        if (d3d11Framebuffer->HasDepthAttachment())
        {
            const auto& viewSpecification{ d3d11Framebuffer->GetDepthAttachmentViewSpecification() };
            switch (d3d11FramebufferSpec.AttachmentsType)
            {
            case TextureType::Texture2D:
                depthStencilView = AsRef<D3D11Texture2D>(d3d11Framebuffer->GetDepthAttachment())->GetD3D11DepthStencilView(viewSpecification).Get();
                break;
            case TextureType::TextureCube:
                depthStencilView = AsRef<D3D11TextureCube>(d3d11Framebuffer->GetDepthAttachment())->GetD3D11DepthStencilView(viewSpecification).Get();
                break;
            default:
                DL_ASSERT(false, "Framebuffer [{0}] has invalid attachments type", d3d11FramebufferSpec.DebugName);
                break;
            }
        }

        d3d11DeviceContext->OMSetRenderTargets(colorAttachmentCount, renderTargetViews.data(), depthStencilView);

        if (clearAttachments)
        {
            for (uint32_t i{ 0u }; i < colorAttachmentCount; ++i)
                d3d11DeviceContext->ClearRenderTargetView(renderTargetViews[i], &d3d11FramebufferSpec.ClearColor.x);

            if (depthStencilView)
            {
                d3d11DeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, d3d11FramebufferSpec.DepthClearValue, 0u);
            }
        }
    }

    void D3D11Renderer::SetMaterial(const Ref<Material>& material) noexcept
    {
        const auto& d3d11Material{ AsRef<D3D11Material>(material) };

        const auto& materialCBs{ d3d11Material->GetConstantBuffers() };
        const auto& materialCBsShaderStages{ d3d11Material->GetConstantBuffersShaderStages() };

        if (!materialCBs.empty())
        {
            auto it{ materialCBs.begin() };
            uint32_t prevBindingPoint{ it->first };
            uint8_t prevShaderStageFlags{ materialCBsShaderStages.at(prevBindingPoint) };
            uint32_t startBindingPoint{ prevBindingPoint };
            std::vector<Ref<ConstantBuffer>> constantBuffers{ it->second };

            ++it;
            for (; it != materialCBs.end(); ++it)
            {
                const uint32_t bindingPoint{ it->first };
                const uint8_t shaderStageFlags{ materialCBsShaderStages.at(bindingPoint) };
                const auto& cb{ it->second };

                if (bindingPoint != prevBindingPoint + 1u || shaderStageFlags != prevShaderStageFlags)
                {
                    SetConstantBuffers(startBindingPoint, prevShaderStageFlags, constantBuffers);
                    constantBuffers.clear();
                    startBindingPoint = bindingPoint;
                }

                constantBuffers.push_back(cb);
                prevBindingPoint = bindingPoint;
            }

            SetConstantBuffers(startBindingPoint, prevShaderStageFlags, constantBuffers);
        }

        const auto& materialTexture2Ds{ d3d11Material->GetTexture2Ds() };
        const auto& materialTextureCubes{ d3d11Material->GetTextureCubes() };
        const auto& materialTexturesShaderStages{ d3d11Material->GetTextureShaderStages() };
        const auto& materialTextureViews{ d3d11Material->GetTextureViews() };

        if (!materialTexture2Ds.empty())
        {
            auto it{ materialTexture2Ds.begin() };
            uint32_t prevBindingPoint{ it->first };
            uint8_t prevShaderStageFlags{ materialTexturesShaderStages.at(prevBindingPoint) };
            uint32_t startBindingPoint{ prevBindingPoint };
            std::vector<Ref<Texture2D>> textures{ it->second };
            std::vector<TextureViewSpecification> viewSpecifications{ materialTextureViews.at(prevBindingPoint) };

            ++it;
            for (; it != materialTexture2Ds.end(); ++it)
            {
                const uint32_t bindingPoint{ it->first };
                const uint8_t shaderStageFlags{ materialTexturesShaderStages.at(bindingPoint) };
                const auto& texture{ it->second };
                const auto& viewSpecification{ materialTextureViews.at(bindingPoint) };

                if (bindingPoint != prevBindingPoint + 1u || shaderStageFlags != prevShaderStageFlags)
                {
                    SetTexture2Ds(startBindingPoint, prevShaderStageFlags, textures, viewSpecifications);
                    textures.clear();
                    viewSpecifications.clear();
                    startBindingPoint = bindingPoint;
                }

                textures.push_back(texture);
                viewSpecifications.push_back(viewSpecification);
                prevBindingPoint = bindingPoint;
            }

            SetTexture2Ds(startBindingPoint, prevShaderStageFlags, textures, viewSpecifications);
        }

        if (!materialTextureCubes.empty())
        {
            auto it{ materialTextureCubes.begin() };
            uint32_t prevBindingPoint{ it->first };
            uint8_t prevShaderStageFlags{ materialTexturesShaderStages.at(prevBindingPoint) };
            uint32_t startBindingPoint{ prevBindingPoint };
            std::vector<Ref<TextureCube>> textures{ it->second };
            std::vector<TextureViewSpecification> viewSpecifications{ materialTextureViews.at(prevBindingPoint) };

            ++it;
            for (; it != materialTextureCubes.end(); ++it)
            {
                const uint32_t bindingPoint{ it->first };
                const uint8_t shaderStageFlags{ materialTexturesShaderStages.at(bindingPoint) };
                const auto& texture{ it->second };
                const auto& viewSpecification{ materialTextureViews.at(bindingPoint) };

                if (bindingPoint != prevBindingPoint + 1u || shaderStageFlags != prevShaderStageFlags)
                {
                    SetTextureCubes(startBindingPoint, prevShaderStageFlags, textures, viewSpecifications);
                    textures.clear();
                    viewSpecifications.clear();
                    startBindingPoint = bindingPoint;
                }

                textures.push_back(texture);
                viewSpecifications.push_back(viewSpecification);
                prevBindingPoint = bindingPoint;
            }

            SetTextureCubes(startBindingPoint, prevShaderStageFlags, textures, viewSpecifications);
        }
    }

    void D3D11Renderer::SubmitStaticMeshInstanced(const Ref<Mesh>& mesh, uint32_t submeshIndex, const Ref<VertexBuffer>& instanceBuffer, uint32_t instanceCount) noexcept
    {
        const auto& d3d11DeviceContext{ D3D11Context::Get()->GetDeviceContext4() };

        const auto& d3d11VertexBuffer{ AsRef<D3D11VertexBuffer>(mesh->GetVertexBuffer()) };
        const auto& d3d11IndexBuffer{ AsRef<D3D11IndexBuffer>(mesh->GetIndexBuffer()) };
        const auto& d3d11InstanceBuffer{ AsRef<D3D11VertexBuffer>(instanceBuffer) };

        std::array<ID3D11Buffer*, 2u> d3d11VertexBuffers{
            d3d11VertexBuffer->GetD3D11VertexBuffer().Get(),
            d3d11InstanceBuffer->GetD3D11VertexBuffer().Get()
        };
        const std::array<uint32_t, 2u> strides{ static_cast<uint32_t>(Mesh::GetCommonVertexBufferLayout().GetStride()), 64u };
        const std::array<uint32_t, 2u> offsets{ 0u, 0u };
        d3d11DeviceContext->IASetVertexBuffers(0u, 2u, d3d11VertexBuffers.data(), strides.data(), offsets.data());
        d3d11DeviceContext->IASetIndexBuffer(d3d11IndexBuffer->GetD3D11IndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0u);

        const auto& submeshRange{ mesh->GetRanges()[submeshIndex] };

        d3d11DeviceContext->DrawIndexedInstanced(
            submeshRange.IndexCount,
            instanceCount,
            submeshRange.IndexOffset,
            submeshRange.VertexOffset,
            0u
        );
    }

    void D3D11Renderer::SubmitFullscreenQuad() noexcept
    {
        const auto& d3d11DeviceContext{ D3D11Context::Get()->GetDeviceContext4() };
        d3d11DeviceContext->Draw(3u, 0u);
    }

    Microsoft::WRL::ComPtr<ID3D11SamplerState> D3D11Renderer::GetSamplerState(const SamplerSpecification& specification)
    {
        const auto it{ s_Data->SamplersCache.find(specification) };
        if (it != s_Data->SamplersCache.end())
            return it->second;

        const auto& address{ Utils::D3D11AddressFromTextureAddress(specification.Address) };

        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter = Utils::D3D11FilterFromTextureFilter(specification.Filter);
        samplerDesc.AddressU = address;
        samplerDesc.AddressV = address;
        samplerDesc.AddressW = address;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = specification.Filter == TextureFilter::Anisotropic8 ? 8u : 1u;
        samplerDesc.ComparisonFunc = Utils::D3D11ComparisonFuncFromCompareOp(specification.CompareOp);
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateSamplerState(&samplerDesc, &samplerState));

        s_Data->SamplersCache[specification] = samplerState;

        return samplerState;
    }

    Microsoft::WRL::ComPtr<ID3D11RasterizerState2> D3D11Renderer::GetRasterizerState(bool backFaceCulling)
    {
        const auto it{ s_Data->RasterizerStatesCache.find(backFaceCulling) };
        if (it != s_Data->RasterizerStatesCache.end())
            return it->second;

        D3D11_RASTERIZER_DESC2 rasterizerDesc{};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = backFaceCulling ? D3D11_CULL_BACK : D3D11_CULL_NONE;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = TRUE;
        rasterizerDesc.ScissorEnable = FALSE;
        rasterizerDesc.MultisampleEnable = FALSE;
        rasterizerDesc.AntialiasedLineEnable = FALSE;
        rasterizerDesc.ForcedSampleCount = 0u;
        rasterizerDesc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> rasterizerState{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateRasterizerState2(&rasterizerDesc, & rasterizerState));

        s_Data->RasterizerStatesCache[backFaceCulling] = rasterizerState;

        return rasterizerState;
    }

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> D3D11Renderer::GetDepthStencilState(const DepthStencilSpecification& specification)
    {
        const auto it{ s_Data->DepthStencilStatesCache.find(specification) };
        if (it != s_Data->DepthStencilStatesCache.end())
            return it->second;

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = specification.DepthTest ? TRUE : FALSE;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = Utils::D3D11ComparisonFuncFromCompareOp(specification.CompareOp);
        depthStencilDesc.StencilEnable = FALSE;

        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState{};
        DL_THROW_IF_HR(D3D11Context::Get()->GetDevice5()->CreateDepthStencilState(&depthStencilDesc, &depthStencilState));

        s_Data->DepthStencilStatesCache[specification] = depthStencilState;

        return depthStencilState;
    }

    void D3D11Renderer::SetShaderResourceViews(uint32_t startSlot, uint8_t shaderStageFlags, const std::vector<ID3D11ShaderResourceView*>& srvs) noexcept
    {
        const auto& d3d11DeviceContext{ D3D11Context::Get()->GetDeviceContext4() };

        if (shaderStageFlags & ShaderStage::DL_VERTEX_SHADER_BIT)
            d3d11DeviceContext->VSSetShaderResources(startSlot, static_cast<UINT>(srvs.size()), srvs.data());

        if (shaderStageFlags & ShaderStage::DL_PIXEL_SHADER_BIT)
            d3d11DeviceContext->PSSetShaderResources(startSlot, static_cast<UINT>(srvs.size()), srvs.data());

        if (shaderStageFlags & ShaderStage::DL_HULL_SHADER_BIT)
            d3d11DeviceContext->HSSetShaderResources(startSlot, static_cast<UINT>(srvs.size()), srvs.data());

        if (shaderStageFlags & ShaderStage::DL_DOMAIN_SHADER_BIT)
            d3d11DeviceContext->DSSetShaderResources(startSlot, static_cast<UINT>(srvs.size()), srvs.data());

        if (shaderStageFlags & ShaderStage::DL_GEOMETRY_SHADER_BIT)
            d3d11DeviceContext->GSSetShaderResources(startSlot, static_cast<UINT>(srvs.size()), srvs.data());

        if (shaderStageFlags & ShaderStage::DL_COMPUTE_SHADER_BIT)
            d3d11DeviceContext->CSSetShaderResources(startSlot, static_cast<UINT>(srvs.size()), srvs.data());
    }

    namespace Utils
    {
        namespace
        {
            D3D11_TEXTURE_ADDRESS_MODE D3D11AddressFromTextureAddress(TextureAddress address)
            {
                switch (address)
                {
                case DLEngine::TextureAddress::Wrap:  return D3D11_TEXTURE_ADDRESS_WRAP;
                case DLEngine::TextureAddress::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
                case DLEngine::TextureAddress::None:
                default: DL_ASSERT(false); return D3D11_TEXTURE_ADDRESS_WRAP;
                }
            }

            D3D11_FILTER D3D11FilterFromTextureFilter(TextureFilter filter)
            {
                switch (filter)
                {
                case DLEngine::TextureFilter::Nearest:      return D3D11_FILTER_MIN_MAG_MIP_POINT;
                case DLEngine::TextureFilter::Trilinear:    return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                case DLEngine::TextureFilter::Anisotropic8: return D3D11_FILTER_ANISOTROPIC;
                case DLEngine::TextureFilter::None:
                default: DL_ASSERT(false); return D3D11_FILTER_MIN_MAG_MIP_POINT;
                }
            }
        }
    }
}