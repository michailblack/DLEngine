#pragma once
#include "DLEngine/Math/Vec3.h"
#include "DLEngine/Utils/DeltaTime.h"

namespace DLEngine
{
    class Camera;
    class Texture2D;

    struct RendererSettings
    {
        Math::Vec3 IndirectLightRadiance{ 0.1f };
        uint32_t UseIBL{ static_cast<uint32_t>(true) };
        uint32_t OverwriteRoughness{ static_cast<uint32_t>(false) };
        float OverwrittenRoughness{ 0.5f };
        uint32_t DiffuseReflections{ static_cast<uint32_t>(true) };
        uint32_t SpecularReflections{ static_cast<uint32_t>(true) };
        float EV100{ 0.0f };
        float _padding[3u];
    };

    class Renderer
    {
    public:
        static void Init();

        static void OnResize(uint32_t width, uint32_t height);

        static void BeginFrame(DeltaTime dt);
        static void EndFrame();

        static void BeginScene(const Camera& camera);
        static void EndScene();

        static void SetRendererSettings(const RendererSettings& settings);
        static void SetSkybox(const Texture2D& skybox);

    private:
        static void InitSkyboxPipeline() noexcept;
        static void DrawSkybox() noexcept;
    };
}
