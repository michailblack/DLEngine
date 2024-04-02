#include "dlpch.h"
#include "Renderer.h"

#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/Events/EventBus.h"

void Renderer::Clear(Math::Vec<3, uint8_t> color)
{
    auto& framebuffer = Application::Get()->GetWindow().GetFramebuffer();

    for (auto& pixel : framebuffer)
        pixel = RGB(color[0], color[1], color[2]);
}

void Renderer::RenderScene(const Scene& scene)
{
    const auto& entities = scene.GetEntities();
    auto& framebuffer = Application::Get()->GetWindow().GetFramebuffer();

    auto [width, height] = Application::Get()->GetWindow().GetSize().Data;
    const uint32_t framebufferWidth = width / 2;
    const uint32_t framebufferHeight = height / 2;

    struct
    {
        float Left   { -10.0f  };
        float Right  {  10.0f  };
        float Bottom { -10.0f  };
        float Top    {  10.0f  };
        float Near   {  0.1f   };
        float Far    {  100.0f };   // Not used
    } static constexpr OrthoFrustum;

    static const Math::Vec3f BL { OrthoFrustum.Left, OrthoFrustum.Bottom, OrthoFrustum.Near };
    static const Math::Vec3f BR { OrthoFrustum.Right, OrthoFrustum.Bottom, OrthoFrustum.Near };
    static const Math::Vec3f TL { OrthoFrustum.Left, OrthoFrustum.Top, OrthoFrustum.Near };

    for (uint32_t y = 0; y < framebufferHeight; ++y)
    {
        for (uint32_t x = 0; x < framebufferWidth; ++x)
        {
            Math::Ray ray {};
            ray.Origin = BL + (BR - BL) * (static_cast<float>(x) / static_cast<float>(framebufferWidth)) + (TL - BL) * (static_cast<float>(y) / static_cast<float>(framebufferHeight));
            ray.Direction = Math::Vec3f { 0.0f, 0.0f, 1.0f };

            for (const auto& entity : entities)
            {
                if (Math::Intersects(ray, entity))
                    framebuffer[y * framebufferWidth + x] = RGB(255, 0, 255);
            }
        }
    }
}

void Renderer::SwapFramebuffer()
{
    PAINTSTRUCT ps;
    const HDC hdc = BeginPaint(Application::Get()->GetWindow().GetHandle(), &ps);

    auto [width, height] = Application::Get()->GetWindow().GetSize().Data;
    const uint32_t framebufferWidth = width / 2;
    const uint32_t framebufferHeight = height / 2;

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
        Application::Get()->GetWindow().GetFramebuffer().data(), &bmi, DIB_RGB_COLORS, SRCCOPY
    );

    EndPaint(Application::Get()->GetWindow().GetHandle(), &ps);
}
