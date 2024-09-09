#include "dlpch.h"
#include "LightSystem.h"

#include "DLEngine/Core/SolidVector.h"

#include "DLEngine/DirectX/ConstantBuffer.h"

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
// Ignore warning C4324: Padding was added at the end of a structure because you specified an alignment specifier
#pragma warning(push)
#pragma warning(disable: 4324)
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

        struct alignas(16) GPUSpotLight
        {
            SpotLight Light;
            uint32_t TransformIndex;
        };

        struct alignas(16) LightsCount
        {
            uint32_t DirectionalLightsCount{ 0u };
            uint32_t PointLightsCount{ 0u };
            uint32_t SpotLightsCount{ 0u };
        };
#pragma warning(pop)

        struct
        {
            SolidVector<DirectionalLightInstance> DirectionalLights;
            SolidVector<PointLightInstance> PointLights;
            SolidVector<SpotLightInstance> SpotLights;

            ConstantBuffer<GPUDirectionalLight> DirectionalLightCB;
            ConstantBuffer<GPUPointLight> PointLightCB;
            ConstantBuffer<GPUSpotLight> SpotLightCB;

            ConstantBuffer<LightsCount> LightsCountCB;
        } s_Data;
    }

    void LightSystem::Init()
    {
        s_Data.DirectionalLightCB.Create(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT / (sizeof(GPUDirectionalLight) / sizeof(float)));
        s_Data.PointLightCB.Create(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT / (sizeof(GPUPointLight) / sizeof(float)));
        s_Data.SpotLightCB.Create(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT / (sizeof(GPUSpotLight) / sizeof(float)));
        s_Data.LightsCountCB.Create();

        DL_LOG_INFO("Light System Initialized");
    }

    uint32_t LightSystem::AddDirectionalLight(const DirectionalLight& light, uint32_t transformID) noexcept
    {
        DirectionalLightInstance lightInst{};
        lightInst.Light = light;
        lightInst.TransformID = transformID;

        const uint32_t id{ s_Data.DirectionalLights.insert(lightInst) };
        return id;
    }

    uint32_t LightSystem::AddPointLight(const PointLight& light, uint32_t transformID) noexcept
    {
        PointLightInstance lightInst{};
        lightInst.Light = light;
        lightInst.TransformID = transformID;

        const uint32_t id{ s_Data.PointLights.insert(lightInst) };
        return id;
    }

    uint32_t LightSystem::AddSpotLight(const SpotLight& light, uint32_t transformID) noexcept
    {
        SpotLightInstance lightInst{};
        lightInst.Light = light;
        lightInst.TransformID = transformID;

        const uint32_t id{ s_Data.SpotLights.insert(lightInst) };
        return id;
    }

    void LightSystem::Update()
    {
        SolidVector<GPUDirectionalLight> gpuDirectionalLights{};
        gpuDirectionalLights.reserve(s_Data.DirectionalLights.size());
        for (const auto& [directionalLight, transformID] : s_Data.DirectionalLights)
        {
            GPUDirectionalLight gpuLight{};
            gpuLight.Light = directionalLight;
            gpuLight.TransformIndex = TransformSystem::GetArrayIndex(transformID);

            gpuDirectionalLights.insert(gpuLight);
        }

        SolidVector<GPUPointLight> gpuPointLights{};
        gpuPointLights.reserve(s_Data.PointLights.size());
        for (const auto& [pointLight, transformID] : s_Data.PointLights)
        {
            GPUPointLight gpuLight{};
            gpuLight.Light = pointLight;
            gpuLight.TransformIndex = TransformSystem::GetArrayIndex(transformID);

            gpuPointLights.insert(gpuLight);
        }

        SolidVector<GPUSpotLight> gpuSpotLights{};
        gpuSpotLights.reserve(s_Data.SpotLights.size());
        for (const auto& [spotLight, transformID] : s_Data.SpotLights)
        {
            GPUSpotLight gpuLight{};
            gpuLight.Light = spotLight;
            gpuLight.TransformIndex = TransformSystem::GetArrayIndex(transformID);

            gpuSpotLights.insert(gpuLight);
        }

        LightsCount lightsCount{};
        lightsCount.DirectionalLightsCount = gpuDirectionalLights.size();
        lightsCount.PointLightsCount = gpuPointLights.size();
        lightsCount.SpotLightsCount = gpuSpotLights.size();

        if (gpuDirectionalLights.size() > 0u)
            s_Data.DirectionalLightCB.Set(gpuDirectionalLights.data(), gpuDirectionalLights.size());
        s_Data.DirectionalLightCB.Bind(5u, BIND_PS);

        if (gpuPointLights.size() > 0u)
            s_Data.PointLightCB.Set(gpuPointLights.data(), gpuPointLights.size());
        s_Data.PointLightCB.Bind(6u, BIND_PS);

        if (gpuSpotLights.size() > 0u)
            s_Data.SpotLightCB.Set(gpuSpotLights.data(), gpuSpotLights.size());
        s_Data.SpotLightCB.Bind(7u, BIND_PS);

        s_Data.LightsCountCB.Set(&lightsCount, 1u);
        s_Data.LightsCountCB.Bind(8u, BIND_PS);
    }
}