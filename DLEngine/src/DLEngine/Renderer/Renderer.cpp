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

        Math::Mat4x4 InvViewProjectionMatrix;
        Math::Vec3 CameraPosition;
    } s_Data;
}

void Renderer::BeginScene(const Camera& camera)
{
    s_Data.InvViewProjectionMatrix = Math::Mat4x4::Inverse(camera.GetViewMatrix() * camera.GetProjectionMatrix());
    s_Data.CameraPosition = camera.GetPosition();
}

void Renderer::EndScene()
{
}

void Renderer::Submit(const Math::Sphere& sphere)
{
    Math::Vec4 BL = Math::Vec4 { -1.0f, -1.0f, 0.0f, 1.0f } * s_Data.InvViewProjectionMatrix;
    BL /= BL.w;

    Math::Vec4 BR = Math::Vec4 {  1.0f, -1.0f, 0.0f, 1.0f } * s_Data.InvViewProjectionMatrix;
    BR /= BR.w;

    Math::Vec4 TL = Math::Vec4 { -1.0f,  1.0f, 0.0f, 1.0f } * s_Data.InvViewProjectionMatrix;
    TL /= TL.w;

    const Math::Vec4 Up = TL - BL;  
    const Math::Vec4 Right = BR - BL;

    for (uint32_t y = 0; y < s_Data.FramebufferHeight; ++y)
    {
        for (uint32_t x = 0; x < s_Data.FramebufferWidth; ++x)
        {
            Math::Ray ray {};

            Math::Vec4 P = BL + Right * (static_cast<float>(x) / static_cast<float>(s_Data.FramebufferWidth))
                + Up * (1.0f - static_cast<float>(y) / static_cast<float>(s_Data.FramebufferHeight));

            ray.Origin = Math::Vec3 { P.x, P.y, P.z };
            ray.Direction = Math::Normalize(ray.Origin - s_Data.CameraPosition);

            Math::IntersectInfo intersectInfo {};

            if (Math::Intersects(ray, sphere, intersectInfo))
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
    bmi.bmiHeader.biHeight = -static_cast<LONG>(s_Data.FramebufferHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(hdc,
        0, 0, static_cast<int>(width), static_cast<int>(height),
        0, 0, static_cast<int>(s_Data.FramebufferWidth), static_cast<int>(s_Data.FramebufferHeight),
        s_Data.Framebuffer.data(), &bmi, DIB_RGB_COLORS, SRCCOPY
    );
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    s_Data.FramebufferWidth = width / s_Data.FramebufferSizeCoefficient;
    s_Data.FramebufferHeight = height / s_Data.FramebufferSizeCoefficient;
    s_Data.Framebuffer.resize(s_Data.FramebufferWidth * s_Data.FramebufferHeight);
}
