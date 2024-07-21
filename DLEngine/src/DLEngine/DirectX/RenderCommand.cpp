#include "dlpch.h"
#include "RenderCommand.h"

#include "DLEngine/DirectX/ConstantBuffer.h"
#include "DLEngine/DirectX/D3DStates.h"
#include "DLEngine/DirectX/IndexBuffer.h"
#include "DLEngine/DirectX/InputLayout.h"
#include "DLEngine/DirectX/PipelineState.h"
#include "DLEngine/DirectX/Shaders.h"
#include "DLEngine/DirectX/StructuredBuffer.h"
#include "DLEngine/DirectX/SwapChain.h"
#include "DLEngine/DirectX/Texture.h"
#include "DLEngine/DirectX/VertexBuffer.h"
#include "DLEngine/DirectX/View.h"

namespace DLEngine
{
    void RenderCommand::SetRenderTargets(const std::initializer_list<RenderTargetView>& RTVs,
        const DepthStencilView& DSV) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        std::vector<ID3D11RenderTargetView*> rtvs{};
        rtvs.reserve(RTVs.size());
        for (const auto& rtv : RTVs)
            rtvs.push_back(static_cast<ID3D11RenderTargetView*>(rtv.m_Handle.Get()));

        deviceContext->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(), DSV.m_Handle.Get());
    }

    void RenderCommand::SetShaderResources(uint32_t startSlot, ShaderStage stage, const std::initializer_list<ShaderResourceView>& SRVs) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        std::array<ID3D11ShaderResourceView*, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> srvs{};

        uint32_t i{ 0u };
        for (const auto& srv : SRVs)
            srvs[i++] = static_cast<ID3D11ShaderResourceView*>(srv.m_Handle.Get());

        switch (stage)
        {
        case ShaderStage::Vertex:
            deviceContext->VSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            break;
        case ShaderStage::Domain:
            deviceContext->DSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            break;
        case ShaderStage::Hull:
            deviceContext->HSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            break;
        case ShaderStage::Geometry:
            deviceContext->GSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            break;
        case ShaderStage::Pixel:
            deviceContext->PSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            break;
        case ShaderStage::Compute:
            deviceContext->CSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            break;
        case ShaderStage::All:
            deviceContext->VSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            deviceContext->DSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            deviceContext->HSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            deviceContext->GSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            deviceContext->PSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            deviceContext->CSSetShaderResources(startSlot, static_cast<UINT>(SRVs.size()), srvs.data());
            break;
        }
    }

    void RenderCommand::SetViewports(const std::initializer_list<D3D11_VIEWPORT>& viewports) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.begin());
    }

    void RenderCommand::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->IASetPrimitiveTopology(topology);
    }

    void RenderCommand::SetInputLayout(const InputLayout& IL) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->IASetInputLayout(IL.m_Handle.Get());
    }

    void RenderCommand::SetVertexBuffers(uint32_t startSlot, const std::initializer_list<VertexBuffer>& VBs) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        std::vector<ID3D11Buffer*> buffers{};
        std::vector<uint32_t> strides{};
        std::vector<uint32_t> offsets{};

        buffers.reserve(VBs.size());
        strides.reserve(VBs.size());
        offsets.reserve(VBs.size());

        for (const auto& vb : VBs)
        {
            buffers.push_back(vb.m_Handle.Get());
            strides.push_back(vb.m_BufferLayout.GetStride());
            offsets.push_back(0u);
        }

        deviceContext->IASetVertexBuffers(startSlot,
            static_cast<UINT>(VBs.size()),
            buffers.data(),
            strides.data(),
            offsets.data()
        );
    }

    void RenderCommand::SetIndexBuffer(const IndexBuffer& IB) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->IASetIndexBuffer(IB.m_Handle.Get(), DXGI_FORMAT_R32_UINT, 0u);
    }

    void RenderCommand::SetConstantBuffers(uint32_t startSlot, ShaderStage stage,
        const std::initializer_list<ConstantBuffer>& CBs) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        std::array<ID3D11Buffer*, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> cbs{};
        uint32_t i{ 0u };
        for (const auto& cb : CBs)
            cbs[i++] = cb.m_Handle.Get();

        switch (stage)
        {
        case ShaderStage::Vertex:
            deviceContext->VSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            break;
        case ShaderStage::Domain:
            deviceContext->DSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            break;
        case ShaderStage::Hull:
            deviceContext->HSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            break;
        case ShaderStage::Geometry:
            deviceContext->GSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            break;
        case ShaderStage::Pixel:
            deviceContext->PSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            break;
        case ShaderStage::Compute:
            deviceContext->CSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            break;
        case ShaderStage::All:
            deviceContext->VSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            deviceContext->DSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            deviceContext->HSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            deviceContext->GSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            deviceContext->PSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            deviceContext->CSSetConstantBuffers1(startSlot, static_cast<UINT>(CBs.size()), cbs.data(), nullptr, nullptr);
            break;
        }
    }

    void RenderCommand::SetVertexShader(const VertexShader& VS) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->VSSetShader(VS.m_Handle.Get(), nullptr, 0u);
    }

    void RenderCommand::SetHullShader(const HullShader& HS) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->HSSetShader(HS.m_Handle.Get(), nullptr, 0u);
    }

    void RenderCommand::SetDomainShader(const DomainShader& DS) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->DSSetShader(DS.m_Handle.Get(), nullptr, 0u);
    }

    void RenderCommand::SetGeometryShader(const GeometryShader& GS) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->GSSetShader(GS.m_Handle.Get(), nullptr, 0u);
    }

    void RenderCommand::SetPixelShader(const PixelShader& PS) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->PSSetShader(PS.m_Handle.Get(), nullptr, 0u);
    }

    void RenderCommand::SetRasterizerState(const RasterizerState& RS) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->RSSetState(RS.m_Handle.Get());
    }

    void RenderCommand::SetDepthStencilState(const DepthStencilState& DSS) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->OMSetDepthStencilState(DSS.m_Handle.Get(), 0u);
    }

    void RenderCommand::SetSamplers(uint32_t startSlot, ShaderStage stage,
        const std::initializer_list<SamplerState>& SSs) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        std::array<ID3D11SamplerState*, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> samplers{};
        uint32_t i{ 0u };
        for (const auto& ss : SSs)
            samplers[i++] = ss.m_Handle.Get();

        switch (stage)
        {
        case ShaderStage::Vertex:
            deviceContext->VSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            break;
        case ShaderStage::Domain:
            deviceContext->DSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            break;
        case ShaderStage::Hull:
            deviceContext->HSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            break;
        case ShaderStage::Geometry:
            deviceContext->GSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            break;
        case ShaderStage::Pixel:
            deviceContext->PSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            break;
        case ShaderStage::Compute:
            deviceContext->CSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            break;
        case ShaderStage::All:
            deviceContext->VSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            deviceContext->DSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            deviceContext->HSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            deviceContext->GSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            deviceContext->PSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            deviceContext->CSSetSamplers(startSlot, static_cast<UINT>(SSs.size()), samplers.data());
            break;
        }
    }

    void RenderCommand::SetPipelineState(const PipelineState& PS) noexcept
    {
        RenderCommand::SetPrimitiveTopology(PS.Topology);

        RenderCommand::SetInputLayout(PS.Layout);

        RenderCommand::SetVertexShader(PS.VS);
        RenderCommand::SetHullShader(PS.HS);
        RenderCommand::SetDomainShader(PS.DS);
        RenderCommand::SetGeometryShader(PS.GS);
        RenderCommand::SetPixelShader(PS.PS);

        RenderCommand::SetRasterizerState(PS.Rasterizer);
        RenderCommand::SetDepthStencilState(PS.DepthStencil);
    }

    void RenderCommand::ClearRenderTargetView(const RenderTargetView& RTV, const Math::Vec4& color) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->ClearRenderTargetView(RTV.m_Handle.Get(), color.data());
    }

    void RenderCommand::ClearDepthStencilView(const DepthStencilView& DSV, float depth, uint8_t stencil) noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        deviceContext->ClearDepthStencilView(DSV.m_Handle.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }

    void RenderCommand::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        DL_THROW_IF_D3D11(deviceContext->Draw(vertexCount, startVertexLocation));
    }

    void RenderCommand::DrawInstancedIndexed(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };
        DL_THROW_IF_D3D11(deviceContext->DrawIndexedInstanced(
            indexCountPerInstance,
            instanceCount,
            startIndexLocation,
            baseVertexLocation,
            startInstanceLocation
        ));
    }

    Texture2D RenderCommand::GetBackBuffer(const SwapChain& swapChain)
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D1> backBuffer{};
        DL_THROW_IF_HR(swapChain.m_Handle->GetBuffer(0, __uuidof(ID3D11Texture2D1), &backBuffer));

        Texture2D texture{};
        texture.Create(backBuffer);
        return texture;
    }

    ShaderResourceView RenderCommand::CreateShaderResourceView(const Texture2D& texture)
    {
        const auto& device{ D3D::GetDevice5() };

        const auto& desc{ texture.GetDesc() };
        D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) ? 
            D3D11_SRV_DIMENSION_TEXTURECUBE :
            D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0u;
        srvDesc.Texture2D.MipLevels = static_cast<UINT>(-1);
        srvDesc.Texture2D.PlaneSlice = 0u;
        
        ShaderResourceView srv{};
        DL_THROW_IF_HR(device->CreateShaderResourceView1(texture.m_Handle.Get(), &srvDesc, &srv.m_Handle));

        return srv;
    }

    ShaderResourceView RenderCommand::CreateShaderResourceView(const StructuredBuffer& structuredBuffer)
    {
        const auto& device{ D3D::GetDevice5() };

        D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc{};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0u;
        srvDesc.Buffer.NumElements = structuredBuffer.m_Count;

        ShaderResourceView srv{};
        DL_THROW_IF_HR(device->CreateShaderResourceView1(structuredBuffer.m_Handle.Get(), &srvDesc, &srv.m_Handle));

        return srv;
    }

    RenderTargetView RenderCommand::CreateRenderTargetView(const Texture2D& texture)
    {
        const auto& device{ D3D::GetDevice5() };

        RenderTargetView rtv{};
        DL_THROW_IF_HR(device->CreateRenderTargetView1(texture.m_Handle.Get(), nullptr, &rtv.m_Handle));

        return rtv;
    }

    DepthStencilView RenderCommand::CreateDepthStencilView(const Texture2D& texture)
    {
        const auto& device{ D3D::GetDevice5() };

        DepthStencilView dsv{};
        DL_THROW_IF_HR(device->CreateDepthStencilView(texture.m_Handle.Get(), nullptr, &dsv.m_Handle));

        return dsv;
    }

}
