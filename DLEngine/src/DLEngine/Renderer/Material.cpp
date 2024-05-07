#include "dlpch.h"
#include "Material.h"

namespace DLEngine
{
    COLORREF Material::CalculateLight(const Math::Ray& ray, const Math::IntersectInfo& intersectionInfo, const Environment& environment) const
    {
        constexpr float threshold{ 1e-3f };
        Math::Vec3 fragmentColor{ environment.IndirectLightingColor * m_Albedo };

        Math::Vec3 diffuse{ 0.0f };
        Math::Vec3 halfwayDir{ 0.0f };
        Math::Vec3 specular{ 0.0f };

        // Calculate direct lighting
        if (environment.Sun.Color != Math::Vec3{ 0.0f })
        {
            const Math::Vec3 directLightDir = -environment.Sun.Direction;
            diffuse = Math::Max(Math::Dot(directLightDir, intersectionInfo.Normal), threshold) * environment.Sun.Color * m_Albedo;
            halfwayDir = Math::Normalize(directLightDir - ray.Direction);
            specular = Math::Vec3{ Math::Pow(Math::Max(Math::Dot(halfwayDir, intersectionInfo.Normal), threshold), m_Glossiness) };

            fragmentColor += diffuse + specular;
        }

        // Calculate pointlight contribution
        for (const auto& pointLight : environment.PointLights)
        {
            Math::Vec3 lightDirection = Math::Normalize(pointLight.Position - intersectionInfo.IntersectionPoint);

            diffuse = Math::Max(Math::Dot(lightDirection, intersectionInfo.Normal), threshold) * pointLight.Color * m_Albedo;

            halfwayDir = Math::Normalize(lightDirection - ray.Direction);
            specular = Math::Vec3{ Math::Pow(Math::Max(Math::Dot(halfwayDir, intersectionInfo.Normal), threshold), m_Glossiness) };

            const float dist = Math::Length(pointLight.Position - intersectionInfo.IntersectionPoint);
            const float attenuation = 1.0f / (1.0f + pointLight.Linear * dist + pointLight.Quadratic * dist * dist);

            fragmentColor += (diffuse + specular) * attenuation;
        }

        // Calculate spotlight contribution
        for (const auto& spotLight : environment.SpotLights)
        {
            Math::Vec3 lightDirection = Math::Normalize(spotLight.Position - intersectionInfo.IntersectionPoint);

            const float angle = Math::Dot(lightDirection, -spotLight.Direction);
            if (angle > spotLight.OuterCutoffCos)
            {
                diffuse = Math::Max(Math::Dot(lightDirection, intersectionInfo.Normal), threshold) * spotLight.Color * m_Albedo;

                halfwayDir = Math::Normalize(lightDirection - ray.Direction);
                specular = Math::Vec3{ Math::Pow(Math::Max(Math::Dot(halfwayDir, intersectionInfo.Normal), threshold), m_Glossiness) };

                const float dist = Math::Length(spotLight.Position - intersectionInfo.IntersectionPoint);
                const float attenuation = 1.0f / (1.0f + spotLight.Linear * dist + spotLight.Quadratic * dist * dist);

                const float spotlightFactor = spotLight.InnerCutoffCos - spotLight.OuterCutoffCos;
                const float intensity = Math::Clamp((angle - spotLight.OuterCutoffCos) / spotlightFactor, 0.0f, 1.0f);

                fragmentColor += (diffuse + specular) * attenuation * intensity;
            }
        }

        return HDRToCOLORREF(fragmentColor, environment.Exposure);
    }

    COLORREF Material::HDRToCOLORREF(const Math::Vec3& hdrColor, float exposure) noexcept
    {
        Math::Vec3 color = Math::Vec3(1.0f) - Math::Exp(-hdrColor * exposure);

        // Gamma correction
        constexpr float gamma{ 2.2f };
        color = Math::Pow(color, Math::Vec3{ 1.0f / gamma });

        // Some weird stuff going on with RGB macro, so I'm swapping the x and z values
        return RGB(color.z * 255, color.y * 255, color.x * 255);
    }
}
