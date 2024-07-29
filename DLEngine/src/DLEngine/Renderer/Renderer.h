#pragma once
#include "DLEngine/Utils/DeltaTime.h"

namespace DLEngine
{
    class Camera;
    class Texture2D;
    struct PostProcessSettings;

    class Renderer
    {
    public:
        static void Init();

        static void OnResize(uint32_t width, uint32_t height);

        static void BeginFrame(DeltaTime dt);
        static void EndFrame();

        static void BeginScene(const Camera& camera);
        static void EndScene();

        static void SetSkybox(const Texture2D& skybox);

        static void SetPostProcessSettings(const PostProcessSettings& settings) noexcept;

    private:
        static void InitSkyboxPipeline() noexcept;
        static void DrawSkybox() noexcept;
    };
}
