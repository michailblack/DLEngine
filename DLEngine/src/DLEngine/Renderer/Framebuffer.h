#pragma once
#include "DLEngine/Core/Base.h"

#include "DLEngine/Math/Vec4.h"

#include "DLEngine/Renderer/Texture.h"

#include <map>

namespace DLEngine
{
    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification(TextureFormat format, TextureUsage usage, uint32_t mips = 1u, uint32_t layers = 1u)
            : Format(format), Usage(usage), Mips(mips), Layers(layers)
        {}

        TextureFormat Format;
        TextureUsage Usage;
        uint32_t Mips{ 1u };
        uint32_t Layers{ 1u };
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
            : Attachments(attachments)
        {}

        std::vector<FramebufferTextureSpecification> Attachments;
    };

    struct FramebufferSpecification
    {
        std::string DebugName;

        FramebufferAttachmentSpecification Attachments;

        // Instead of creation of attachments, specify existing ones,
        // where key is the index of the attachment in the framebuffer
        std::map<uint32_t, Ref<Texture>> ExistingAttachments;

        Math::Vec4 ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        float DepthClearValue{ 0.0f };

        TextureType AttachmentsType{ TextureType::Texture2D };

        uint32_t Width;
        uint32_t Height;

        uint32_t Samples{ 1u };

        // If true, then Attachments, ExistingAttachments, Width and Height are ignored.
        // Framebuffer is created with back buffer and depth-stencil attachments automatically
        bool SwapChainTarget{ false };
      
        uint8_t StencilClearValue{ 0u };
        uint8_t StencilReferenceValue{ 0u };
    };

    class Framebuffer
    {
    public:
        virtual ~Framebuffer() = default;

        virtual void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) = 0;

        virtual void Invalidate() noexcept = 0;

        virtual void SetColorAttachmentViewSpecification(uint32_t index, const TextureViewSpecification& specification) noexcept = 0;
        virtual void SetDepthAttachmentViewSpecification(const TextureViewSpecification& specification) noexcept = 0;

        virtual uint32_t GetWidth() const noexcept = 0;
        virtual uint32_t GetHeight() const noexcept = 0;

        virtual uint32_t GetColorAttachmentCount() const noexcept = 0;
        virtual Ref<Texture> GetColorAttachment(uint32_t index) const noexcept = 0;
        virtual Ref<Texture> GetDepthAttachment() const noexcept = 0;
        virtual bool HasDepthAttachment() const noexcept = 0;

        virtual const TextureViewSpecification& GetColorAttachmentViewSpecification(uint32_t index) const noexcept = 0;
        virtual const TextureViewSpecification& GetDepthAttachmentViewSpecification() const noexcept = 0;

        virtual const FramebufferSpecification& GetSpecification() const noexcept = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& specification);
    };
}