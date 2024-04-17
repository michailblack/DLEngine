#include "dlpch.h"
#include "Renderer.h"

#include "ThreadPool.h"
#include "DLEngine/Core/Application.h"

#include "DLEngine/Math/Intersections.h"
#include "DLEngine/Math/Math.h"

namespace
{
    struct RenderData
    {
        ThreadPool RenderThreadPool;

        std::vector<COLORREF> Framebuffer;
        const uint32_t FramebufferSizeCoefficient { 3 };
        uint32_t FramebufferWidth { 0 };
        uint32_t FramebufferHeight { 0 };

        Math::Mat4x4 InvViewProjectionMatrix;
        Math::Vec3 CameraPosition;

        std::vector<Ref<SphereInstance>> Spheres;
        std::vector<Ref<PlaneInstance>> Planes;
        std::vector<Ref<MeshInstance>> Cubes;
    } s_Data;
}

void Renderer::Init()
{
    s_Data.RenderThreadPool.Create(std::thread::hardware_concurrency() - 1);
}

void Renderer::BeginScene(const Camera& camera)
{
    s_Data.InvViewProjectionMatrix = Math::Mat4x4::Inverse(camera.GetViewMatrix() * camera.GetProjectionMatrix());
    s_Data.CameraPosition = camera.GetPosition();
}

void Renderer::EndScene()
{
    Math::Vec4 BL = Math::Vec4 { -1.0f, -1.0f, 0.0f, 1.0f } * s_Data.InvViewProjectionMatrix;
    BL /= BL.w;

    Math::Vec4 BR = Math::Vec4 {  1.0f, -1.0f, 0.0f, 1.0f } * s_Data.InvViewProjectionMatrix;
    BR /= BR.w;

    Math::Vec4 TL = Math::Vec4 { -1.0f,  1.0f, 0.0f, 1.0f } * s_Data.InvViewProjectionMatrix;
    TL /= TL.w;

    const Math::Vec4 Up = TL - BL;  
    const Math::Vec4 Right = BR - BL;

    const uint32_t threadCount = s_Data.RenderThreadPool.GetWorkersCountUnsafe();
    const uint32_t threadZoneHeight = s_Data.FramebufferHeight / (threadCount + 1);
    
    for (uint32_t i = 0; i < threadCount; ++i)
    {
        const uint32_t startHeight = i * threadZoneHeight;
        const uint32_t endHeight = (i + 1) * threadZoneHeight;
    
        s_Data.RenderThreadPool.AddTask(RenderPerThread, startHeight, endHeight, BL, Right, Up);
    }
    
    RenderPerThread(threadCount * threadZoneHeight, s_Data.FramebufferHeight, BL, Right, Up);

    s_Data.RenderThreadPool.Wait();

    s_Data.Spheres.clear();
    s_Data.Cubes.clear();

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

void Renderer::Submit(const Ref<SphereInstance>& sphere)
{
    s_Data.Spheres.emplace_back(sphere);
}

void Renderer::Submit(const Ref<PlaneInstance>& plane)
{
    s_Data.Planes.emplace_back(plane);
}

void Renderer::Submit(const Ref<MeshInstance>& cube)
{
    s_Data.Cubes.emplace_back(cube);
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    s_Data.FramebufferWidth = width / s_Data.FramebufferSizeCoefficient;
    s_Data.FramebufferHeight = height / s_Data.FramebufferSizeCoefficient;
    s_Data.Framebuffer.resize(s_Data.FramebufferWidth * s_Data.FramebufferHeight);
}

void Renderer::RenderPerThread(uint32_t startHeight, uint32_t height, const Math::Vec4& BL, const Math::Vec4& Right,
    const Math::Vec4& Up)
{
    for (uint32_t y = startHeight; y < height; ++y)
    {
        for (uint32_t x = 0; x < s_Data.FramebufferWidth; ++x)
        {
            Math::Ray ray {};

            const Math::Vec4 P = BL + Right * (static_cast<float>(x) / static_cast<float>(s_Data.FramebufferWidth))
                + Up * (1.0f - static_cast<float>(y) / static_cast<float>(s_Data.FramebufferHeight));

            ray.Origin = Math::Vec3 { P.x, P.y, P.z };
            ray.Direction = Math::Normalize(ray.Origin - s_Data.CameraPosition);

            Math::IntersectInfo intersectInfo {};

            for (const auto& plane : s_Data.Planes)
            {
                if (Math::Intersects(ray, plane->Plane, intersectInfo))
                {
                    s_Data.Framebuffer[y * s_Data.FramebufferWidth + x] = RGB(100, 100, 100);
                }
            }

            for (const auto& sphere : s_Data.Spheres)
            {
                if (Math::Intersects(ray, sphere->Sphere, intersectInfo))
                {
                    s_Data.Framebuffer[y * s_Data.FramebufferWidth + x] = RGB(255, 0, 0);
                }
            }

            for (const auto& cube : s_Data.Cubes)
            {
                const Math::Vec4 rayOriginModelSpace = Math::Vec4 { ray.Origin, 1.0f } * cube->InvTransform;
                const Math::Vec4 rayDirModelSpace = Math::Vec4 { ray.Direction, 0.0f } * cube->InvTransform;

                Math::Ray rayModelSpace;
                rayModelSpace.Origin = Math::Vec3 { rayOriginModelSpace.x, rayOriginModelSpace.y, rayOriginModelSpace.z };
                rayModelSpace.Direction = Math::Normalize(Math::Vec3 { rayDirModelSpace.x, rayDirModelSpace.y, rayDirModelSpace.z });

                Math::IntersectInfo intersectInfoModelSpace {};
                if (intersectInfo.Step != Math::Infinity())
                {
                    const Math::Vec4 intersectionPointModelSpace = Math::Vec4 { intersectInfo.IntersectionPoint, 1.0f } * cube->InvTransform;
                    intersectInfoModelSpace.IntersectionPoint = Math::Vec3 { intersectionPointModelSpace.x, intersectionPointModelSpace.y, intersectionPointModelSpace.z };
                    intersectInfoModelSpace.Step = Math::Length(intersectInfoModelSpace.IntersectionPoint - rayModelSpace.Origin);
                }
                const float stepBefore = intersectInfoModelSpace.Step;

                for (const auto& triangle : Mesh::GetUnitCube().GetTriangles())
                {
                    if (Math::Intersects(rayModelSpace, triangle, intersectInfoModelSpace))
                    {
                        
                    }
                }

                if (intersectInfoModelSpace.Step != stepBefore)
                {
                    s_Data.Framebuffer[y * s_Data.FramebufferWidth + x] = RGB(0, 0, 255);

                    const Math::Vec4 intersectionPointWorldSpace = Math::Vec4 { intersectInfoModelSpace.IntersectionPoint, 1.0f } * cube->Transform;
                    const Math::Vec4 normal = Math::Normalize(Math::Vec4 { intersectInfoModelSpace.Normal.x, intersectInfoModelSpace.Normal.y, intersectInfoModelSpace.Normal.z, 0.0f } * cube->Transform);

                    intersectInfo.IntersectionPoint = Math::Vec3 { intersectionPointWorldSpace.x, intersectionPointWorldSpace.y, intersectionPointWorldSpace.z };
                    intersectInfo.Step = Math::Length(intersectInfo.IntersectionPoint - ray.Origin);
                    intersectInfo.Normal = Math::Vec3 { normal.x, normal.y, normal.z };
                }
            }

            if (intersectInfo.Step == Math::Infinity())
                s_Data.Framebuffer[y * s_Data.FramebufferWidth + x] = RGB(0, 0, 0);
        }
    }
}
