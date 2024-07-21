#include "dlpch.h"
#include "LightSystem.h"

#include "DLEngine/Core/solid_vector.h"

#include "DLEngine/DirectX/RenderCommand.h"
#include "DLEngine/DirectX/StructuredBuffer.h"

#include "DLEngine/Systems/Transform/TransformSystem.h"

namespace DLEngine
{
    namespace 
    {
        struct DirectionalLightInstance
        {
            DirectionalLight Light;
            uint32_t TransformID;
        };

        struct PointLightInstance
        {
            PointLight Light;
            uint32_t TransformID;
        };

        struct SpotLightInstance
        {
            SpotLight Light;
            uint32_t TransformID;
        };

        struct GPUDirectionalLight
        {
            DirectionalLight Light;
            uint32_t TransformIndex;
        };

        struct GPUPointLight
        {
            PointLight Light;
            uint32_t TransformIndex;
        };

        struct GPUSpotLight
        {
            SpotLight Light;
            uint32_t TransformIndex;
        };

        struct
        {
            solid_vector<DirectionalLightInstance> DirectionalLights;
            solid_vector<PointLightInstance> PointLights;
            solid_vector<SpotLightInstance> SpotLights;

            RStructuredBuffer DirectionalLightSB;
            RStructuredBuffer PointLightSB;
            RStructuredBuffer SpotLightSB;

            bool ShouldUpdate{ false };
        } s_Data;
    }

    void LightSystem::Init()
    {
        StructuredBufferDesc desc{};

        desc.StructureSize = sizeof(GPUDirectionalLight);
        StructuredBuffer sb{};
        sb.Create(desc);
        s_Data.DirectionalLightSB.Create(sb);

        desc.StructureSize = sizeof(GPUPointLight);
        sb.Reset();
        sb.Create(desc);
        s_Data.PointLightSB.Create(sb);

        desc.StructureSize = sizeof(GPUSpotLight);
        sb.Reset();
        sb.Create(desc);
        s_Data.SpotLightSB.Create(sb);

        DL_LOG_INFO("Light System Initialized");
    }

    uint32_t LightSystem::AddDirectionalLight(const DirectionalLight& light, uint32_t transformID) noexcept
    {
        s_Data.ShouldUpdate = true;

        DirectionalLightInstance lightInst{};
        lightInst.Light = light;
        lightInst.TransformID = transformID;

        const uint32_t id{ s_Data.DirectionalLights.insert(lightInst) };
        return id;
    }

    uint32_t LightSystem::AddPointLight(const PointLight& light, uint32_t transformID) noexcept
    {
        s_Data.ShouldUpdate = true;

        PointLightInstance lightInst{};
        lightInst.Light = light;
        lightInst.TransformID = transformID;

        const uint32_t id{ s_Data.PointLights.insert(lightInst) };
        return id;
    }

    uint32_t LightSystem::AddSpotLight(const SpotLight& light, uint32_t transformID) noexcept
    {
        s_Data.ShouldUpdate = true;

        SpotLightInstance lightInst{};
        lightInst.Light = light;
        lightInst.TransformID = transformID;

        const uint32_t id{ s_Data.SpotLights.insert(lightInst) };
        return id;
    }

    uint32_t LightSystem::GetPointLightArrayIndex(uint32_t id) noexcept
    {
        return s_Data.PointLights.getIndex(id);
    }

    void LightSystem::Update()
    {
        if (s_Data.ShouldUpdate)
        {
            s_Data.ShouldUpdate = false;

            uint32_t i{ 0u };

            if (s_Data.DirectionalLights.capacity() != s_Data.DirectionalLightSB.GetStructuredBuffer().GetElementsCount())
                s_Data.DirectionalLightSB.Resize(s_Data.DirectionalLights.capacity());
            auto* directionalLights{ static_cast<GPUDirectionalLight*>(s_Data.DirectionalLightSB.GetStructuredBuffer().Map()) };
            for (const auto& [directionalLight, transformID] : s_Data.DirectionalLights)
            {
                GPUDirectionalLight gpuLight{};
                gpuLight.Light = directionalLight;
                gpuLight.TransformIndex = TransformSystem::GetArrayIndex(transformID);

                directionalLights[i++] = gpuLight;
            }
            s_Data.DirectionalLightSB.GetStructuredBuffer().Unmap();

            i = 0u;
            if (s_Data.PointLights.capacity() != s_Data.PointLightSB.GetStructuredBuffer().GetElementsCount())
                s_Data.PointLightSB.Resize(s_Data.PointLights.capacity());
            auto* pointLights{ static_cast<GPUPointLight*>(s_Data.PointLightSB.GetStructuredBuffer().Map()) };
            for (const auto& [pointLight, transformID] : s_Data.PointLights)
            {
                GPUPointLight gpuLight{};
                gpuLight.Light = pointLight;
                gpuLight.TransformIndex = TransformSystem::GetArrayIndex(transformID);

                pointLights[i++] = gpuLight;
            }
            s_Data.PointLightSB.GetStructuredBuffer().Unmap();

            i = 0u;
            if (s_Data.SpotLights.capacity() != s_Data.SpotLightSB.GetStructuredBuffer().GetElementsCount())
                s_Data.SpotLightSB.Resize(s_Data.SpotLights.capacity());
            auto* spotLights{ static_cast<GPUSpotLight*>(s_Data.SpotLightSB.GetStructuredBuffer().Map()) };
            for (const auto& [spotLight, transformID] : s_Data.SpotLights)
            {
                GPUSpotLight gpuLight{};
                gpuLight.Light = spotLight;
                gpuLight.TransformIndex = TransformSystem::GetArrayIndex(transformID);

                spotLights[i++] = gpuLight;
            }
            s_Data.SpotLightSB.GetStructuredBuffer().Unmap();
        }

        RenderCommand::SetShaderResources(
            2u,
            ShaderStage::All,
            {
                s_Data.DirectionalLightSB.GetSRV(),
                s_Data.PointLightSB.GetSRV(),
                s_Data.SpotLightSB.GetSRV()
            }
        );
    }
}