#pragma once
#include "DLEngine/DirectX/RenderCommand.h"
#include "DLEngine/DirectX/Texture.h"

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
            Texture2D Texture{};

            void Set() const noexcept
            {
                RenderCommand::SetShaderResources(8u, ShaderStage::Pixel, { Texture.GetSRV() });
            }

            bool operator==(const TextureOnly& other) const
            {
                return Texture == other.Texture;
            }
        };

        struct Lit
        {
            Texture2D Albedo{};
            Texture2D Normal{};
            Texture2D Metallic{};
            Texture2D Roughness{};

            void Set() const noexcept
            {
                RenderCommand::SetShaderResources(
                    8u,
                    ShaderStage::Pixel,
                    {
                        Albedo.GetSRV(),
                        Normal.GetSRV(),
                        Metallic.GetSRV(),
                        Roughness.GetSRV()
                    }
                );
            }

            bool operator==(const Lit& other) const
            {
                return Albedo == other.Albedo &&
                    Normal == other.Normal &&
                    Metallic == other.Metallic &&
                    Roughness == other.Roughness;
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
