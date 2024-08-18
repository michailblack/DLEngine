#include "dlpch.h"
#include "Framebuffer.h"

#include "DLEngine/DirectX/D3D11Framebuffer.h"

namespace DLEngine
{
    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& specification)
    {
        return CreateRef<D3D11Framebuffer>(specification);
    }
}