#include "dlpch.h"
#include "D3DStates.h"

namespace std
{
    template<>
    struct hash<D3D11_DEPTH_STENCIL_DESC>
    {
        size_t operator()(const D3D11_DEPTH_STENCIL_DESC& desc) const noexcept
        {
            const char* ptr{ reinterpret_cast<const char*>(&desc) };
            size_t size{ sizeof(D3D11_DEPTH_STENCIL_DESC) };

            return std::hash<std::string_view>{}(std::string_view{ ptr, size });
        }
    };

    template<>
    struct hash<D3D11_RASTERIZER_DESC2>
    {
        size_t operator()(const D3D11_RASTERIZER_DESC2& desc) const noexcept
        {
            const char* ptr{ reinterpret_cast<const char*>(&desc) };
            size_t size{ sizeof(D3D11_RASTERIZER_DESC2) };

            return std::hash<std::string_view>{}(std::string_view{ ptr, size });
        }
    };

    template<>
    struct hash<D3D11_SAMPLER_DESC>
    {
        size_t operator()(const D3D11_SAMPLER_DESC& desc) const noexcept
        {
            const char* ptr{ reinterpret_cast<const char*>(&desc) };
            size_t size{ sizeof(D3D11_SAMPLER_DESC) };

            return std::hash<std::string_view>{}(std::string_view{ ptr, size });
        }
    };
}

bool operator==(const D3D11_DEPTH_STENCIL_DESC& lhs, const D3D11_DEPTH_STENCIL_DESC& rhs)
{
    return std::memcmp(&lhs, &rhs, sizeof(D3D11_DEPTH_STENCIL_DESC)) == 0;
}

bool operator==(const D3D11_RASTERIZER_DESC2& lhs, const D3D11_RASTERIZER_DESC2& rhs)
{
    return std::memcmp(&lhs, &rhs, sizeof(D3D11_RASTERIZER_DESC2)) == 0;
}

bool operator==(const D3D11_SAMPLER_DESC& lhs, const D3D11_SAMPLER_DESC& rhs)
{
    return std::memcmp(&lhs, &rhs, sizeof(D3D11_SAMPLER_DESC)) == 0;
}

namespace DLEngine
{
    namespace Utils
    {
        namespace
        {
            D3D11_DEPTH_STENCIL_DESC DepthStencilDescFromEnum(DepthStencilStates state)
            {
                D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
                
                switch (state)
                {
                case DepthStencilStates::DEFAULT:
                {
                    depthStencilDesc.DepthEnable = TRUE;
                    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
                    depthStencilDesc.StencilEnable = FALSE;

                    return depthStencilDesc;
                }
                case DepthStencilStates::DEPTH_READ_ONLY:
                {
                    depthStencilDesc.DepthEnable = TRUE;
                    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
                    depthStencilDesc.StencilEnable = FALSE;

                    return depthStencilDesc;
                }
                case DepthStencilStates::DEPTH_DISABLED:
                {
                    depthStencilDesc.DepthEnable = FALSE;
                    depthStencilDesc.StencilEnable = FALSE;

                    return depthStencilDesc;
                }
                default:
                    DL_ASSERT(false, "Unknown DepthStencilState");
                    return depthStencilDesc;
                }
            }

            D3D11_RASTERIZER_DESC2 RasterizerDescFromEnum(RasterizerStates state)
            {
                D3D11_RASTERIZER_DESC2 rasterizerDesc{};
                
                switch (state)
                {
                case RasterizerStates::DEFAULT:
                {
                    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
                    rasterizerDesc.CullMode = D3D11_CULL_BACK;
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

                    return rasterizerDesc;
                }
                default:
                    DL_ASSERT(false, "Unknown RasterizerState");
                    return rasterizerDesc;
                }
            }

            D3D11_SAMPLER_DESC SmaplerDescFromEnum(SamplerStates state)
            {
                D3D11_SAMPLER_DESC samplerDesc{};
                samplerDesc.MipLODBias = 0.0f;
                samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
                samplerDesc.MinLOD = 0.0f;
                samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

                switch (state)
                {
                case SamplerStates::POINT_WRAP:
                {
                    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
                    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
                    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
                    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

                    return samplerDesc;
                }
                case SamplerStates::POINT_CLAMP:
                {
                    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
                    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
                    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
                    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                 
                    return samplerDesc;
                }
                case SamplerStates::TRILINEAR_WRAP:
                {
                    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
                    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
                    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
                 
                    return samplerDesc;
                }
                case SamplerStates::TRILINEAR_CLAMP:
                {
                    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
                    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
                    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                 
                    return samplerDesc;
                }
                case SamplerStates::ANISOTROPIC_8_WRAP:
                {
                    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
                    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
                    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
                    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
                    samplerDesc.MaxAnisotropy = 8u;
                    
                    return samplerDesc;
                }
                case SamplerStates::ANISOTROPIC_8_CLAMP:
                {
                    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
                    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
                    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
                    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                    samplerDesc.MaxAnisotropy = 8u;
                 
                    return samplerDesc;
                }
                default:
                    DL_ASSERT(false, "Unknown SamplerState");
                    return {};
                }
            }
        }
    }

    namespace
    {
        struct D3DStatesData
        {
            std::unordered_map<D3D11_RASTERIZER_DESC2, RasterizerState> RasterizerStates;
            std::unordered_map<D3D11_DEPTH_STENCIL_DESC, DepthStencilState> DepthStencilStates;
            std::unordered_map<D3D11_SAMPLER_DESC, SamplerState> SamplerStates;
        } s_Data;
    }

    void RasterizerState::Create(const D3D11_RASTERIZER_DESC2& desc)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateRasterizerState2(&desc, &Handle));
    }

    void RasterizerState::Bind() const noexcept
    {
        D3D::GetDeviceContext4()->RSSetState(Handle.Get());
    }

    void DepthStencilState::Create(const D3D11_DEPTH_STENCIL_DESC& desc)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateDepthStencilState(&desc, &Handle));
    }

    void DepthStencilState::Bind() const noexcept
    {
        D3D::GetDeviceContext4()->OMSetDepthStencilState(Handle.Get(), 0u);
    }

    void SamplerState::Create(const D3D11_SAMPLER_DESC& desc)
    {
        DL_THROW_IF_HR(D3D::GetDevice5()->CreateSamplerState(&desc, &Handle));
    }

    void SamplerState::Bind(uint32_t slot, uint8_t shaderBindFlags) const noexcept
    {
        const auto& deviceContext{ D3D::GetDeviceContext4() };

        if (shaderBindFlags & BIND_VS)
            deviceContext->VSSetSamplers(slot, 1u, Handle.GetAddressOf());

        if (shaderBindFlags & BIND_HS)
            deviceContext->HSSetSamplers(slot, 1u, Handle.GetAddressOf());

        if (shaderBindFlags & BIND_DS)
            deviceContext->DSSetSamplers(slot, 1u, Handle.GetAddressOf());

        if (shaderBindFlags & BIND_GS)
            deviceContext->GSSetSamplers(slot, 1u, Handle.GetAddressOf());

        if (shaderBindFlags & BIND_PS)
            deviceContext->PSSetSamplers(slot, 1u, Handle.GetAddressOf());
    }

    RasterizerState D3DStates::GetRasterizerState(const D3D11_RASTERIZER_DESC2& desc)
    {
        auto it{ s_Data.RasterizerStates.find(desc) };
        if (it != s_Data.RasterizerStates.end())
            return it->second;

        RasterizerState rasterizerState{};
        rasterizerState.Create(desc);

        s_Data.RasterizerStates[desc] = rasterizerState;

        return rasterizerState;
    }

    RasterizerState D3DStates::GetRasterizerState(RasterizerStates state)
    {
        return GetRasterizerState(Utils::RasterizerDescFromEnum(state));
    }

    DepthStencilState D3DStates::GetDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc)
    {
        auto it{ s_Data.DepthStencilStates.find(desc) };
        if (it != s_Data.DepthStencilStates.end())
            return it->second;

        DepthStencilState depthStencilState{};
        depthStencilState.Create(desc);

        s_Data.DepthStencilStates[desc] = depthStencilState;

        return depthStencilState;
    }

    DepthStencilState D3DStates::GetDepthStencilState(DepthStencilStates state)
    {
        return GetDepthStencilState(Utils::DepthStencilDescFromEnum(state));
    }

    SamplerState D3DStates::GetSamplerState(const D3D11_SAMPLER_DESC& desc)
    {
        auto it{ s_Data.SamplerStates.find(desc) };
        if (it != s_Data.SamplerStates.end())
            return it->second;

        SamplerState samplerState{};
        samplerState.Create(desc);

        s_Data.SamplerStates[desc] = samplerState;

        return samplerState;
    }

    SamplerState D3DStates::GetSamplerState(SamplerStates state)
    {
        return GetSamplerState(Utils::SmaplerDescFromEnum(state));
    }
}
