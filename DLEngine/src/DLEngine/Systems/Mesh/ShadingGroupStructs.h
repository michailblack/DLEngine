#pragma once
#include "DLEngine/DirectX/RenderCommand.h"
#include "DLEngine/DirectX/View.h"

#include "DLEngine/Math/Vec3.h"

#include "DLEngine/Systems/Light/LightSystem.h"

namespace DLEngine
{
    namespace ShadingGroupStruct::Material
    {
        struct Null
        {
            void Set() const noexcept {}
            bool operator==(const Null&) const { return true; }
        };

        struct TextureOnly
        {
            ShaderResourceView TextureSRV{};

            void Set() const noexcept
            {
                RenderCommand::SetShaderResources(5u, ShaderStage::Pixel, { TextureSRV });
            }

            bool operator==(const TextureOnly& other) const
            {
                return TextureSRV == other.TextureSRV;
            }
        };

        struct Lit
        {
            ShaderResourceView AlbedoSRV{};
            ShaderResourceView NormalSRV{};
            ShaderResourceView MetallicSRV{};
            ShaderResourceView RoughnessSRV{};

            void Set() const noexcept
            {
                RenderCommand::SetShaderResources(5u, ShaderStage::Pixel, { AlbedoSRV, NormalSRV, MetallicSRV, RoughnessSRV });
            }

            bool operator==(const Lit& other) const
            {
                return AlbedoSRV == other.AlbedoSRV &&
                    NormalSRV == other.NormalSRV &&
                    MetallicSRV == other.MetallicSRV &&
                    RoughnessSRV == other.RoughnessSRV;
            }
        };
    }

    namespace ShadingGroupStruct::Instance
    {
        struct Null
        {
            uint32_t TransformID{ 0u };

            struct GPUType
            {
                uint32_t TransformIndex{ 0u };
            };

            GPUType ConvertToGPU() const
            {
                GPUType gpuInstance{};
                gpuInstance.TransformIndex = TransformSystem::GetArrayIndex(TransformID);

                return gpuInstance;
            }
        };

        struct Hologram
        {
            Math::Vec3 BaseColor{};
            Math::Vec3 AdditionalColor{};
            uint32_t TransformID{ 0u };

            struct GPUType
            {
                Math::Vec3 BaseColor{};
                Math::Vec3 AdditionalColor{};
                uint32_t TransformIndex{ 0u };
            };

            GPUType ConvertToGPU() const
            {
                GPUType gpuInstance{};
                gpuInstance.BaseColor = BaseColor;
                gpuInstance.AdditionalColor = AdditionalColor;
                gpuInstance.TransformIndex = TransformSystem::GetArrayIndex(TransformID);

                return gpuInstance;
            }
        };

        struct Emission
        {
            uint32_t PointLightID{ 0u };
            uint32_t TransformID{ 0u };

            struct GPUType
            {
                uint32_t PointLightIndex{ 0u };
                uint32_t TransformIndex{ 0u };
            };

            GPUType ConvertToGPU() const
            {
                GPUType gpuInstance{};
                gpuInstance.PointLightIndex = LightSystem::GetPointLightArrayIndex(PointLightID);
                gpuInstance.TransformIndex = TransformSystem::GetArrayIndex(TransformID);

                return gpuInstance;
            }
        };
    }
}
