#include "dlpch.h"
#include "D3D11Framebuffer.h"

#include "DLEngine/DirectX/D3D11Context.h"

#include "DLEngine/Renderer/Renderer.h"

#include <d3d11_4.h>

namespace DLEngine
{
    D3D11Framebuffer::D3D11Framebuffer(const FramebufferSpecification& specification)
        : m_Specification(specification)
    {
        if (m_Specification.SwapChainTarget)
            CreateFramebufferForSwapChain();
        else if (m_Specification.ExistingAttachments.empty())
            CreateAttachments();
        else
            ProcessExistingAttachments();
    }

    void D3D11Framebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
    {
        if (m_Specification.Width == width && m_Specification.Height == height && !forceRecreate)
            return;

        m_Specification.Width = width;
        m_Specification.Height = height;

        Invalidate();

        if (m_Specification.SwapChainTarget)
            CreateFramebufferForSwapChain();
        else
            CreateAttachments();
    }

    void D3D11Framebuffer::Invalidate() noexcept
    {
        m_ColorAttachments.clear();
        m_DepthAttachment.reset();

        m_ColorAttachmentViewSpecifications.clear();
        m_DepthAttachmentViewSpecification = TextureViewSpecification{};
    }

    void D3D11Framebuffer::SetColorAttachmentViewSpecification(uint32_t index, const TextureViewSpecification& specification) noexcept
    {
        DL_ASSERT(index < static_cast<uint32_t>(m_ColorAttachments.size()),
            "Framebuffer [{0}] has no color attachment at index {1}", m_Specification.DebugName, index
        );

        m_ColorAttachmentViewSpecifications[index] = specification;
    }

    void D3D11Framebuffer::SetDepthAttachmentViewSpecification(const TextureViewSpecification& specification) noexcept
    {
        DL_ASSERT(m_DepthAttachment,
            "Framebuffer [{0}] has no depth attachment", m_Specification.DebugName
        );

        m_DepthAttachmentViewSpecification = specification;
    }

    Ref<Texture> D3D11Framebuffer::GetColorAttachment(uint32_t index) const noexcept
    {
        DL_ASSERT(index < static_cast<uint32_t>(m_ColorAttachments.size()),
            "Framebuffer [{0}] has no color attachment at index {1}", m_Specification.DebugName, index
        );
        return m_ColorAttachments[index];
    }

    const TextureViewSpecification& D3D11Framebuffer::GetColorAttachmentViewSpecification(uint32_t index) const noexcept
    {
        DL_ASSERT(index < static_cast<uint32_t>(m_ColorAttachments.size()),
            "Framebuffer [{0}] has no color attachment at index {1}", m_Specification.DebugName, index
        );

        return m_ColorAttachmentViewSpecifications[index];
    }

    const TextureViewSpecification& D3D11Framebuffer::GetDepthAttachmentViewSpecification() const noexcept
    {
        DL_ASSERT(m_DepthAttachment,
            "Framebuffer [{0}] has no depth attachment", m_Specification.DebugName
        );

        return m_DepthAttachmentViewSpecification;
    }

    void D3D11Framebuffer::CreateFramebufferForSwapChain()
    {
        m_Specification.Attachments.Attachments.clear();
        m_Specification.ExistingAttachments.clear();
        m_Specification.AttachmentsType = TextureType::Texture2D;

        m_ColorAttachments.emplace_back(Renderer::GetBackBufferTexture());
        const auto& backBufferSpec{ m_ColorAttachments[0]->GetSpecification() };

        TextureSpecification depthStencilSpec{};
        depthStencilSpec.DebugName = "Standard depth-stencil attachment";
        depthStencilSpec.Format = TextureFormat::DEPTH24STENCIL8;
        depthStencilSpec.Usage = TextureUsage::Attachment;
        depthStencilSpec.Width = backBufferSpec.Width;
        depthStencilSpec.Height = backBufferSpec.Height;

        m_DepthAttachment = Texture2D::Create(depthStencilSpec);

        m_Specification.Width = backBufferSpec.Width;
        m_Specification.Height = backBufferSpec.Height;

        m_ColorAttachmentViewSpecifications.emplace_back(TextureViewSpecification{});
        m_DepthAttachmentViewSpecification = TextureViewSpecification{};
    }

    void D3D11Framebuffer::CreateAttachments()
    {
        TextureSpecification textureSpec{};
        textureSpec.Width = m_Specification.Width;
        textureSpec.Height = m_Specification.Height;
        textureSpec.Mips = 1u;
        textureSpec.Layers = 1u;

        for (uint32_t i{ 0u }; i < m_Specification.Attachments.Attachments.size(); ++i)
        {
            const auto& attachmentSpec = m_Specification.Attachments.Attachments[i];

            DL_ASSERT(attachmentSpec.Usage != TextureUsage::None && attachmentSpec.Usage != TextureUsage::Texture,
                "Framebuffer [{0}] has invalid attachment usage", m_Specification.DebugName
            );

            textureSpec.Usage = attachmentSpec.Usage;
            textureSpec.Format = attachmentSpec.Format;
            textureSpec.Sampler = attachmentSpec.Sampler;

            if (Utils::IsDepthFormat(attachmentSpec.Format))
            {
                DL_ASSERT(!m_DepthAttachment,
                    "Framebuffer [{0}] has more than one depth attachment", m_Specification.DebugName
                );
                textureSpec.DebugName = m_Specification.DebugName + " Depth Attachment " + std::to_string(i);
                switch (m_Specification.AttachmentsType)
                {
                case TextureType::Texture2D:
                    m_DepthAttachment = Texture2D::Create(textureSpec);
                    break;
                case TextureType::TextureCube:
                    m_DepthAttachment = TextureCube::Create(textureSpec);
                    break;
                case TextureType::None:
                default:
                    DL_ASSERT(false, "Framebuffer [{0}] has invalid attachments type", m_Specification.DebugName);
                    break;
                }

                m_DepthAttachmentViewSpecification = TextureViewSpecification{};
            }
            else
            {
                DL_ASSERT(m_ColorAttachments.size() < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
                    "Framebuffer [{0}] has too many color attachments", m_Specification.DebugName
                );
                textureSpec.DebugName = m_Specification.DebugName + " Color Attachment " + std::to_string(i);
                switch (m_Specification.AttachmentsType)
                {
                case TextureType::Texture2D:
                    m_ColorAttachments.emplace_back(Texture2D::Create(textureSpec));
                    break;
                case TextureType::TextureCube:
                    m_ColorAttachments.emplace_back(TextureCube::Create(textureSpec));
                    break;
                case TextureType::None:
                default:
                    DL_ASSERT(false, "Framebuffer [{0}] has invalid attachments type", m_Specification.DebugName);
                    break;
                }

                m_ColorAttachmentViewSpecifications.emplace_back(TextureViewSpecification{});
            }
        }
    }

    void D3D11Framebuffer::ProcessExistingAttachments()
    {
        m_Specification.Attachments.Attachments.clear();
        uint32_t prevIndex{ 0u };
        for (const auto& [index, attachment] : m_Specification.ExistingAttachments)
        {
            if (index != prevIndex++)
                DL_LOG_WARN_TAG("Renderer", "Specified indices for attachments for framebuffer [{0}] are not sequential", m_Specification.DebugName);

            const auto& attachmentSpec{ attachment->GetSpecification() };

            DL_ASSERT(attachmentSpec.Usage != TextureUsage::None && attachmentSpec.Usage != TextureUsage::Texture,
                "Framebuffer [{0}] has invalid attachment usage", m_Specification.DebugName
            );

            DL_ASSERT(attachment->GetType() == m_Specification.AttachmentsType,
                "Framebuffer [{0}] has invalid attachments type", m_Specification.DebugName
            );

            if (Utils::IsDepthFormat(attachmentSpec.Format))
            {
                DL_ASSERT(!m_DepthAttachment,
                    "Framebuffer [{0}] has more than one depht attachment", m_Specification.DebugName
                );
                m_DepthAttachment = attachment;

                m_DepthAttachmentViewSpecification = TextureViewSpecification{};
            }
            else
            {
                DL_ASSERT(m_ColorAttachments.size() < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
                    "Framebuffer [{0}] has too many color attachments", m_Specification.DebugName
                );
                m_ColorAttachments.emplace_back(attachment);

                m_ColorAttachmentViewSpecifications.emplace_back(TextureViewSpecification{});
            }

            m_Specification.Attachments.Attachments.emplace_back(attachmentSpec.Format, attachmentSpec.Usage, attachmentSpec.Sampler);
        }
        m_Specification.ExistingAttachments.clear();
    }

}