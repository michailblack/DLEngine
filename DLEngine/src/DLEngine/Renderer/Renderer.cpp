#include "dlpch.h"
#include "Renderer.h"

#include "DLEngine/Core/Application.h"

#include "DLEngine/Math/Intersections.h"
#include "DLEngine/Math/Math.h"

#include "DLEngine/Renderer/Mesh.h"

namespace DLEngine
{
    namespace
    {
        struct RenderData
        {
            Math::Mat4x4 InvViewProjectionMatrix;
            Math::Vec3 CameraPosition;

            std::vector<Ref<SphereInstance>> Spheres;
            std::vector<Ref<PlaneInstance>> Planes;
            std::vector<Ref<MeshInstance>> Cubes;

            Ref<Environment> EnvironmentInfo;

            float LightSphereRadius{ 0.1f };
        } s_Data;
    }

    void Renderer::Init()
    {

    }

    void Renderer::BeginScene(const Camera& camera, const Ref<Environment>& environment)
    {
        s_Data.InvViewProjectionMatrix = Math::Mat4x4::Inverse(camera.GetViewMatrix() * camera.GetProjectionMatrix());
        s_Data.CameraPosition = camera.GetPosition();
        s_Data.EnvironmentInfo = environment;
    }

    void Renderer::EndScene()
    {

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
        Math::Vec4 BL = Math::Vec4{ -1.0f, -1.0f, 0.0f, 1.0f } *s_Data.InvViewProjectionMatrix;
        BL /= BL.w;

        Math::Vec4 BR = Math::Vec4{ 1.0f, -1.0f, 0.0f, 1.0f } *s_Data.InvViewProjectionMatrix;
        BR /= BR.w;

        Math::Vec4 TL = Math::Vec4{ -1.0f,  1.0f, 0.0f, 1.0f } *s_Data.InvViewProjectionMatrix;
        TL /= TL.w;

        const Math::Vec4 Up = TL - BL;
        const Math::Vec4 Right = BR - BL;

        const Math::Vec4 P = BL + Right * (static_cast<float>(mouseX) / Application::Get().GetWindow()->GetSize().x)
            + Up * (1.0f - static_cast<float>(mouseY) / Application::Get().GetWindow()->GetSize().y);

        Math::Ray ray{};
        ray.Origin = P.xyz();
        ray.Direction = Math::Normalize(ray.Origin - s_Data.CameraPosition);

        return ray;
    }

    Environment Renderer::CalculateEnvironmentContribution(const Math::IntersectInfo& intersectionInfo, const Environment& environment)
    {
        constexpr float bias{ 1e-3f };
        Environment resultEnvironment{ environment };

        Math::Ray shadowRay;
        shadowRay.Origin = intersectionInfo.IntersectionPoint + intersectionInfo.Normal * bias;

        shadowRay.Direction = -environment.Sun.Direction;
        if (PointIsOccluded(shadowRay, Math::Infinity()))
            resultEnvironment.Sun.Color = Math::Vec3{ 0.0f };

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
        Math::IntersectInfo intersectInfo{};

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

        /*for (const auto& cube : s_Data.Cubes)
        {
            if (Math::Intersects(ray, *cube, Mesh::GetUnitCube(), intersectInfo))
            {
                if (intersectInfo.T < lightSourceT)
                    return true;
            }
        }*/

        return false;
    }
}
