#pragma once
#include "DLEngine/Renderer/Framebuffer.h"

namespace DLEngine
{
    class D3D11Framebuffer : public Framebuffer
    {
    public:
        D3D11Framebuffer(const FramebufferSpecification& specification);

        void Resize(uint32_t width, uint32_t height, bool forceRecreate) override;

        void Invalidate() noexcept override;

        void SetColorAttachmentViewSpecification(uint32_t index, const TextureViewSpecification& specification) noexcept override;
        void SetDepthAttachmentViewSpecification(const TextureViewSpecification& specification) noexcept override;

        uint32_t GetWidth() const noexcept override { return m_Specification.Width; }
        uint32_t GetHeight() const noexcept override { return m_Specification.Height; }

        uint32_t GetColorAttachmentCount() const noexcept override { return static_cast<uint32_t>(m_ColorAttachments.size()); }
        Ref<Texture> GetColorAttachment(uint32_t index) const noexcept override;
        Ref<Texture> GetDepthAttachment() const noexcept override { return m_DepthAttachment; }
        bool HasDepthAttachment() const noexcept override { return static_cast<bool>(m_DepthAttachment); }

        const TextureViewSpecification& GetColorAttachmentViewSpecification(uint32_t index) const noexcept override;
        const TextureViewSpecification& GetDepthAttachmentViewSpecification() const noexcept override;

        const FramebufferSpecification& GetSpecification() const noexcept override { return m_Specification; }

    private:
        void CreateFramebufferForSwapChain();
        void CreateAttachments();
        void ProcessExistingAttachments();

    private:
        FramebufferSpecification m_Specification;

        std::vector<Ref<Texture>> m_ColorAttachments;
        Ref<Texture> m_DepthAttachment;

        std::vector<TextureViewSpecification> m_ColorAttachmentViewSpecifications;
        TextureViewSpecification m_DepthAttachmentViewSpecification;
    };
}