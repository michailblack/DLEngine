#pragma once
#include "DLEngine/DirectX/ConstantBuffer.h"
#include "DLEngine/DirectX/PipelineState.h"
#include "DLEngine/DirectX/StructuredBuffer.h"
#include "DLEngine/DirectX/Texture.h"

namespace DLEngine
{
    class ReflectionCapture
    {
    public:
        void Create(uint32_t numHemispherePoints);

        void Build(const Texture2D& skybox);

        Texture2D GetDiffuseIrradiance() const noexcept { return m_DiffuseIrradianceMap; }
        Texture2D GetSpecularIrradiance() const noexcept { return m_SpecularIrradianceMap; }
        Texture2D GetSpecularFactor() const noexcept { return m_SpecularFactorMap; }

    private:
        PipelineState m_PipelineState;

        ConstantBuffer m_ConstantBuffer;
        StructuredBuffer m_HemispherePointsSB;

        PixelShader m_DiffuseIrradiancePS;
        Texture2D m_DiffuseIrradianceMap;

        PixelShader m_SpecularIrradiancePS;
        Texture2D m_SpecularIrradianceMap;

        PixelShader m_SpecularFactorPS;
        Texture2D m_SpecularFactorMap;

        uint32_t m_TextureSize{ 0u };
        uint32_t m_NumSpecularMipLevels{ 1u };
    };
}
