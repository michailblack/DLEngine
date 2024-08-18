#pragma once
#include "DLEngine/Renderer/DepthStencil.h"
#include "DLEngine/Renderer/Framebuffer.h"
#include "DLEngine/Renderer/Shader.h"

namespace DLEngine
{
    enum class PrimitiveTopology
    {
        None = 0,
        TrianglesList,
        TriangleStrip,
    };

    struct PipelineSpecification
    {
        std::string DebugName;

        Ref<Shader> Shader;
        Ref<Framebuffer> TargetFramebuffer;
        PrimitiveTopology Topology{ PrimitiveTopology::TrianglesList };
        DepthStencilSpecification DepthStencilState;
        bool BackFaceCulling{ true };        
    };
    
    class Pipeline
    {
    public:
        virtual ~Pipeline() = default;

        virtual const PipelineSpecification& GetSpecification() const noexcept = 0;

        static Ref<Pipeline> Create(const PipelineSpecification& specification);
    };
}