#include "dlpch.h"
#include "DXStates.h"

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
}

bool operator==(const D3D11_DEPTH_STENCIL_DESC& lhs, const D3D11_DEPTH_STENCIL_DESC& rhs)
{
    return std::memcmp(&lhs, &rhs, sizeof(D3D11_DEPTH_STENCIL_DESC)) == 0;
}

bool operator==(const D3D11_RASTERIZER_DESC2& lhs, const D3D11_RASTERIZER_DESC2& rhs)
{
    return std::memcmp(&lhs, &rhs, sizeof(D3D11_RASTERIZER_DESC2)) == 0;
}

namespace DLEngine
{
    using namespace Microsoft::WRL;

    namespace Utils
    {
        namespace
        {
            D3D11_DEPTH_STENCIL_DESC DepthStencilDescFromEnum(DepthStencilStates state)
            {
                switch (state)
                {
                case DepthStencilStates::Default:
                {
                    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
                    depthStencilDesc.DepthEnable = TRUE;
                    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
                    depthStencilDesc.StencilEnable = FALSE;

                    return depthStencilDesc;
                }
                default:
                    DL_ASSERT(false, "Unknown DepthStencilState");
                    return {};
                }
            }

            D3D11_RASTERIZER_DESC2 RasterizerDescFromEnum(RasterizerStates state)
            {
                switch (state)
                {
                case RasterizerStates::Default:
                {
                    D3D11_RASTERIZER_DESC2 rasterizerDesc{};
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
                    return {};
                }
            }
        }
    }

    namespace
    {
        struct
        {
            std::unordered_map<D3D11_DEPTH_STENCIL_DESC, ComPtr<ID3D11DepthStencilState>> DepthStencilStates;
            std::unordered_map<D3D11_RASTERIZER_DESC2, ComPtr<ID3D11RasterizerState2>> RasterizerStates;
        } s_StateData;
    }

    void DXStates::Init()
    {

    }

    ComPtr<ID3D11DepthStencilState> DXStates::GetDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc)
    {
        auto it{ s_StateData.DepthStencilStates.find(desc) };
        if (it != s_StateData.DepthStencilStates.end())
            return it->second;

        const auto& device{ D3D::GetDevice5() };
        ComPtr<ID3D11DepthStencilState> depthStencilState;
        DL_THROW_IF_HR(device->CreateDepthStencilState(&desc, &depthStencilState));

        s_StateData.DepthStencilStates[desc] = depthStencilState;

        return depthStencilState;
    }

    ComPtr<ID3D11DepthStencilState> DXStates::GetDepthStencilState(DepthStencilStates state)
    {
        return GetDepthStencilState(Utils::DepthStencilDescFromEnum(state));
    }

    ComPtr<ID3D11RasterizerState2> DXStates::GetRasterizerState(const D3D11_RASTERIZER_DESC2& desc)
    {
        auto it{ s_StateData.RasterizerStates.find(desc) };
        if (it != s_StateData.RasterizerStates.end())
            return it->second;

        const auto& device{ D3D::GetDevice5() };
        ComPtr<ID3D11RasterizerState2> rasterizerState;
        DL_THROW_IF_HR(device->CreateRasterizerState2(&desc, &rasterizerState))

        s_StateData.RasterizerStates[desc] = rasterizerState;

        return rasterizerState;
    }

    ComPtr<ID3D11RasterizerState2> DXStates::GetRasterizerState(RasterizerStates state)
    {
        return GetRasterizerState(Utils::RasterizerDescFromEnum(state));
    }
}
