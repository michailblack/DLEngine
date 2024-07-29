#pragma once
#include "DLEngine/DirectX/ConstantBuffer.h"
#include "DLEngine/DirectX/PipelineState.h"

namespace DLEngine
{
    class Texture2D;

    struct PostProcessSettings
    {
        float EV100{ 0.0f };
        float Gamma{ 2.2f };
        float _padding[2u]{};
    };

    class PostProcess
    {
    public:
        void Create();

        void SetSettings(const PostProcessSettings& settings) noexcept;
        void Resolve(const Texture2D& src, const Texture2D& dst) const;

    private:
        PipelineState m_PipelineState;

        PostProcessSettings m_CurrentSettings{};
        ConstantBuffer m_SettingsCB{};
    };
}
