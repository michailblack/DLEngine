﻿#include "dlpch.h"
#include "Renderer.h"

namespace
{
    struct RenderData
    {
        struct
        {
            float Left   { -10.0f  };
            float Right  {  10.0f  };
            float Bottom { -10.0f  };
            float Top    {  10.0f  };
            float Near   {  0.1f   };
            float Far    {  100.0f };
        } const OrthoFrustum;

        Ref<Window> Window;
    } s_Data;
}

void Renderer::SetWindow(const Ref<Window>& window)
{
    s_Data.Window = window;
}

Ref<Window>& Renderer::GetWindow()
{
    return s_Data.Window;
}

void Renderer::Draw(const Math::Sphere& entity)
{
    auto& framebuffer = s_Data.Window->GetFramebuffer();

    const auto [framebufferWidth, framebufferHeight] = s_Data.Window->GetFramebufferSize().Data;

    static const Math::Vec3f BL { s_Data.OrthoFrustum.Left , s_Data.OrthoFrustum.Bottom, s_Data.OrthoFrustum.Near };
    static const Math::Vec3f BR { s_Data.OrthoFrustum.Right, s_Data.OrthoFrustum.Bottom, s_Data.OrthoFrustum.Near };
    static const Math::Vec3f TL { s_Data.OrthoFrustum.Left , s_Data.OrthoFrustum.Top   , s_Data.OrthoFrustum.Near };

    for (uint32_t y = 0; y < framebufferHeight; ++y)
    {
        for (uint32_t x = 0; x < framebufferWidth; ++x)
        {
            Math::Ray ray {};
            ray.Origin = BL + (BR - BL) * (static_cast<float>(x) / static_cast<float>(framebufferWidth)) + (TL - BL) * (static_cast<float>(y) / static_cast<float>(framebufferHeight));
            ray.Direction = Math::Vec3f { 0.0f, 0.0f, 1.0f };

            if (Math::Intersects(ray, entity) != std::nullopt)
                framebuffer[y * framebufferWidth + x] = RGB(255, 0, 255);
            else
                framebuffer[y * framebufferWidth + x] = RGB(0, 0, 0);
        }
    }


    auto [width, height] = s_Data.Window->GetSize().Data;
    const HDC hdc = GetDC(s_Data.Window->GetHandle());

    BITMAPINFO bmi {};
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = static_cast<LONG>(framebufferWidth);
    bmi.bmiHeader.biHeight = static_cast<LONG>(framebufferHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(hdc,
        0, 0, static_cast<int>(width), static_cast<int>(height),
        0, 0, static_cast<int>(framebufferWidth), static_cast<int>(framebufferHeight),
        s_Data.Window->GetFramebuffer().data(), &bmi, DIB_RGB_COLORS, SRCCOPY
    );
}

Math::Vec2f Renderer::ScreenSpaceToWorldSpace(const Math::Vec2<int32_t>& screenPos)
{
    const auto [wndWidth, wndHeight] = s_Data.Window->GetSize().Data;

    const float x = static_cast<float>(screenPos.Data[0]) / static_cast<float>(wndWidth) * (s_Data.OrthoFrustum.Right - s_Data.OrthoFrustum.Left) + s_Data.OrthoFrustum.Left;
    const float y = static_cast<float>(screenPos.Data[1]) / static_cast<float>(wndHeight) * (s_Data.OrthoFrustum.Top - s_Data.OrthoFrustum.Bottom) + s_Data.OrthoFrustum.Bottom;

    return { x, y };
}

bool Renderer::MouseHoveringOverEntity(const Math::Vec2<int32_t>& mousePos)
{
    const auto& [wndWidth, wndHeight] = s_Data.Window->GetSize().Data;
    const auto& framebuffer = s_Data.Window->GetFramebuffer();
    const auto& [framebufferWidth, framebufferHeight] = s_Data.Window->GetFramebufferSize().Data;
    
    const uint32_t mouseX = static_cast<uint32_t>(static_cast<float>(mousePos.Data[0]) / static_cast<float>(wndWidth) * static_cast<float>(framebufferWidth));
    const uint32_t mouseY = static_cast<uint32_t>(static_cast<float>(mousePos.Data[1]) / static_cast<float>(wndHeight) * static_cast<float>(framebufferHeight));
    
    return framebuffer[(framebufferHeight - mouseY - 1) * framebufferWidth + mouseX] != RGB(0, 0, 0);
}
