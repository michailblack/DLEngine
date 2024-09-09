#pragma once
#include "DLEngine/Renderer/LightCasters.h"

namespace DLEngine
{
    class LightSystem
    {
    public:
        static void Init();

        static uint32_t AddDirectionalLight(const DirectionalLight& light, uint32_t transformID) noexcept;
        static uint32_t AddPointLight(const PointLight& light, uint32_t transformID) noexcept;
        static uint32_t AddSpotLight(const SpotLight& light, uint32_t transformID) noexcept;

        static uint32_t GetPointLightArrayIndex(uint32_t id) noexcept;

        static void Update();
    };
}
