#include "dlpch.h"
#include "Renderer.h"

#include "DLEngine/Core/Application.h"

namespace
{
    struct RenderData
    {
        std::vector<COLORREF> Framebuffer;
        const uint32_t FramebufferSizeCoefficient { 3 };
        uint32_t FramebufferWidth { 0 };
        uint32_t FramebufferHeight { 0 };

        const float widthMultiplier { 5.0f };
        struct
        {
            float Left   { -5.0f  };
            float Right  {  5.0f  };
            float Bottom { -5.0f  };
            float Top    {  5.0f  };
            float Near   {  0.1f   };
            float Far    {  100.0f };
        } OrthoFrustum;
    } s_Data;
}

void Renderer::Draw(const Math::Sphere& entity)
{
    const Math::Vec3 BL { s_Data.OrthoFrustum.Left , s_Data.OrthoFrustum.Bottom, s_Data.OrthoFrustum.Near };
    const Math::Vec3 BR { s_Data.OrthoFrustum.Right, s_Data.OrthoFrustum.Bottom, s_Data.OrthoFrustum.Near };
    const Math::Vec3 TL { s_Data.OrthoFrustum.Left , s_Data.OrthoFrustum.Top   , s_Data.OrthoFrustum.Near };

    for (uint32_t y = 0; y < s_Data.FramebufferHeight; ++y)
    {
        for (uint32_t x = 0; x < s_Data.FramebufferWidth; ++x)
        {
            Math::Ray ray {};
            ray.Origin = BL + (BR - BL) * (static_cast<float>(x) / static_cast<float>(s_Data.FramebufferWidth)) + (TL - BL) * (static_cast<float>(y) / static_cast<float>(s_Data.FramebufferHeight));
            ray.Direction = { 0.0f, 0.0f, 1.0f };

            Math::IntersectInfo intersectInfo {};

            if (Math::Intersects(ray, entity, intersectInfo))
                s_Data.Framebuffer[y * static_cast<int64_t>(s_Data.FramebufferWidth) + x] = RGB(255, 0, 255);
            else
                s_Data.Framebuffer[y * static_cast<int64_t>(s_Data.FramebufferWidth) + x] = RGB(0, 0, 0);
        }
    }

    auto [width, height] = Application::Get().GetWindow()->GetSize();
    const HDC hdc = GetDC(Application::Get().GetWindow()->GetHandle());

    BITMAPINFO bmi {};
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = static_cast<LONG>(s_Data.FramebufferWidth);
    bmi.bmiHeader.biHeight = static_cast<LONG>(s_Data.FramebufferHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(hdc,
        0, 0, static_cast<int>(width), static_cast<int>(height),
        0, 0, static_cast<int>(s_Data.FramebufferWidth), static_cast<int>(s_Data.FramebufferHeight),
        s_Data.Framebuffer.data(), &bmi, DIB_RGB_COLORS, SRCCOPY
    );
}

Math::Vec2 Renderer::ScreenSpaceToWorldSpace(const Math::Vec2& screenPos)
{
    const auto [wndWidth, wndHeight] = Application::Get().GetWindow()->GetSize();

    const float x = screenPos.x / wndWidth * (s_Data.OrthoFrustum.Right - s_Data.OrthoFrustum.Left) + s_Data.OrthoFrustum.Left;
    const float y = screenPos.y / wndHeight * (s_Data.OrthoFrustum.Top - s_Data.OrthoFrustum.Bottom) + s_Data.OrthoFrustum.Bottom;

    return { x, y };
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    s_Data.OrthoFrustum.Left  = -s_Data.widthMultiplier * aspectRatio;
    s_Data.OrthoFrustum.Right =  s_Data.widthMultiplier * aspectRatio;

    s_Data.FramebufferWidth = width / s_Data.FramebufferSizeCoefficient;
    s_Data.FramebufferHeight = height / s_Data.FramebufferSizeCoefficient;
    s_Data.Framebuffer.resize(s_Data.FramebufferWidth * s_Data.FramebufferHeight);
}
