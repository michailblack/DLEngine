#include "dlpch.h"
#include "Renderer.h"

#include "DLEngine/Core/Application.h"

#include "DLEngine/Math/Intersections.h"
#include "DLEngine/Math/Math.h"

#include "DLEngine/Renderer/Mesh.h"

#include "DLEngine/Utils/ThreadPool.h"

namespace
{
    struct RenderData
    {
        ThreadPool RenderThreadPool;

        std::vector<COLORREF> Framebuffer;
        uint32_t FramebufferSizeCoefficient { 2 };
        uint32_t FramebufferWidth { 0 };
        uint32_t FramebufferHeight { 0 };

        Math::Mat4x4 InvViewProjectionMatrix;
        Math::Vec3 CameraPosition;

        std::vector<Ref<SphereInstance>> Spheres;
        std::vector<Ref<PlaneInstance>> Planes;
        std::vector<Ref<MeshInstance>> Cubes;

        Ref<Environment> EnvironmentInfo;

        float LightSphereRadius { 0.1f };
    } s_Data;
}

void Renderer::Init()
{
    s_Data.RenderThreadPool.Create(std::thread::hardware_concurrency() - 1);
}

void Renderer::BeginScene(const Camera& camera, const Ref<Environment>& environment)
{
    s_Data.InvViewProjectionMatrix = Math::Mat4x4::Inverse(camera.GetViewMatrix() * camera.GetProjectionMatrix());
    s_Data.CameraPosition = camera.GetPosition();
    s_Data.EnvironmentInfo = environment;
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
    s_Data.Planes.clear();
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

Math::Ray Renderer::GetRay(uint32_t mouseX, uint32_t mouseY)
{
    Math::Vec4 BL = Math::Vec4 { -1.0f, -1.0f, 0.0f, 1.0f } * s_Data.InvViewProjectionMatrix;
    BL /= BL.w;

    Math::Vec4 BR = Math::Vec4 {  1.0f, -1.0f, 0.0f, 1.0f } * s_Data.InvViewProjectionMatrix;
    BR /= BR.w;

    Math::Vec4 TL = Math::Vec4 { -1.0f,  1.0f, 0.0f, 1.0f } * s_Data.InvViewProjectionMatrix;
    TL /= TL.w;

    const Math::Vec4 Up = TL - BL;
    const Math::Vec4 Right = BR - BL;

    const Math::Vec4 P = BL + Right * (static_cast<float>(mouseX) / Application::Get().GetWindow()->GetSize().x)
        + Up * (1.0f - static_cast<float>(mouseY) / Application::Get().GetWindow()->GetSize().y);

    Math::Ray ray {};
    ray.Origin = P.xyz();
    ray.Direction = Math::Normalize(ray.Origin - s_Data.CameraPosition);

    return ray;
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    s_Data.FramebufferWidth = width / s_Data.FramebufferSizeCoefficient;
    s_Data.FramebufferHeight = height / s_Data.FramebufferSizeCoefficient;
    s_Data.Framebuffer.resize(s_Data.FramebufferWidth * s_Data.FramebufferHeight);
}

void Renderer::SetFramebufferSizeCoefficient(uint32_t framebufferSizeCoefficient)
{
    std::cout << "FramebufferSizeCoefficient: " << framebufferSizeCoefficient << '\n';
    s_Data.FramebufferSizeCoefficient = framebufferSizeCoefficient;
    OnResize(Application::Get().GetWindow()->GetWidth(), Application::Get().GetWindow()->GeHeigth());
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

            ray.Origin = P.xyz();
            ray.Direction = Math::Normalize(ray.Origin - s_Data.CameraPosition);

            Math::IntersectInfo intersectInfo {};
            auto& fragmentColor = s_Data.Framebuffer[y * s_Data.FramebufferWidth + x];
            const Material* fragmentMaterial { nullptr };

            for (const auto& plane : s_Data.Planes)
            {
                if (Math::Intersects(ray, plane->Plane, intersectInfo))
                {
                    fragmentMaterial = &plane->Mat;
                }
            }

            for (const auto& sphere : s_Data.Spheres)
            {
                if (Math::Intersects(ray, sphere->Sphere, intersectInfo))
                {
                    fragmentMaterial = &sphere->Mat;
                }
            }

            for (const auto& cube : s_Data.Cubes)
            {
                if (Math::Intersects(ray, *cube, Mesh::GetUnitCube(), intersectInfo))
                {
                    fragmentMaterial = &cube->Mat;
                }
            }

            const PointLight* visiblePointLight { nullptr };
            Math::Sphere lightSphere;
            lightSphere.Radius = s_Data.LightSphereRadius;
            for (const auto& pointLight : s_Data.EnvironmentInfo->PointLights)
            {
                lightSphere.Center = pointLight.Position;
                if (Math::Intersects(ray, lightSphere, intersectInfo))
                {
                    visiblePointLight = &pointLight;
                }
            }

            const SpotLight* visibleSpotLight { nullptr };
            for (const auto& spotLight : s_Data.EnvironmentInfo->SpotLights)
            {
                lightSphere.Center = spotLight.Position;
                if (Math::Intersects(ray, lightSphere, intersectInfo))
                {
                    visibleSpotLight = &spotLight;
                }
            }

            if (visibleSpotLight)
            {
                fragmentColor = Material::HDRToCOLORREF(visibleSpotLight->Color, s_Data.EnvironmentInfo->Exposure);
            }
            else if (visiblePointLight)
            {
                fragmentColor = Material::HDRToCOLORREF(visiblePointLight->Color, s_Data.EnvironmentInfo->Exposure);
            }
            else if (fragmentMaterial)
            {
                fragmentColor = fragmentMaterial->CalculateLight(ray, intersectInfo, CalculateEnvironmentContribution(intersectInfo, *s_Data.EnvironmentInfo));
            }
            else
            {
                fragmentColor = Material::HDRToCOLORREF(s_Data.EnvironmentInfo->IndirectLightingColor, s_Data.EnvironmentInfo->Exposure);
            }
        }
    }
}

Environment Renderer::CalculateEnvironmentContribution(const Math::IntersectInfo& intersectionInfo, const Environment& environment)
{
    constexpr float bias { 1e-3f };
    Environment resultEnvironment { environment };

    Math::Ray shadowRay;
    shadowRay.Origin = intersectionInfo.IntersectionPoint + intersectionInfo.Normal * bias;

    shadowRay.Direction = -environment.Sun.Direction;
    if (PointIsOccluded(shadowRay, Math::Infinity()))
        resultEnvironment.Sun.Color = Math::Vec3 { 0.0f };

    std::erase_if(resultEnvironment.PointLights, [&](const PointLight& pointLight)
        {
            shadowRay.Direction = Math::Normalize(pointLight.Position - intersectionInfo.IntersectionPoint);
            return PointIsOccluded(shadowRay, Math::Length(pointLight.Position - intersectionInfo.IntersectionPoint));
        });

    std::erase_if(resultEnvironment.SpotLights, [&](const SpotLight& spotLight)
        {
            shadowRay.Direction = Math::Normalize(spotLight.Position - intersectionInfo.IntersectionPoint);
            return PointIsOccluded(shadowRay, Math::Length(spotLight.Position - intersectionInfo.IntersectionPoint));
        });

    return resultEnvironment;
}

bool Renderer::PointIsOccluded(const Math::Ray& ray, const float lightSourceT)
{
    Math::IntersectInfo intersectInfo {};

    for (const auto& plane : s_Data.Planes)
    {
        if (Math::Intersects(ray, plane->Plane, intersectInfo))
        {
            if (intersectInfo.T < lightSourceT)
                return true;
        }
    }

    for (const auto& sphere : s_Data.Spheres)
    {
        if (Math::Intersects(ray, sphere->Sphere, intersectInfo))
        {
            if (intersectInfo.T < lightSourceT)
                return true;
        }
    }

    for (const auto& cube : s_Data.Cubes)
    {
        if (Math::Intersects(ray, *cube, Mesh::GetUnitCube(), intersectInfo))
        {
            if (intersectInfo.T < lightSourceT)
                return true;
        }
    }

    return false;
}
