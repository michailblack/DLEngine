#ifndef _LIGHTING_HLSLI_
#define _LIGHTING_HLSLI_

#include "Buffers.hlsli"
#include "PBR.hlsli"
#include "PBR_Resources.hlsli"
#include "Samplers.hlsli"
#include "ShadowMapping.hlsli"

struct View
{
    float3 ViewDir;
    float3 ReflectionDir;
    float NoV;
    float NoL;
};

struct Surface
{
    float3 Albedo;
    float3 GeometryNormal;
    float3 SurfaceNormal;
    float3 F0;
    float Metalness;
    float Roughness;
};

struct Light
{
    float3 SpecularLightDir;
    float3 Radiance;
    float SolidAngle;
};

float3 IBL(in const View view, in const Surface surface)
{
    const float3 diffuseIrradianceIBL = t_EnvironmentIrradiance.Sample(s_Anisotropic8Clamp, surface.SurfaceNormal).rgb;

    float width, height, numMipLevels;
    t_PrefilteredEnvironment.GetDimensions(0, width, height, numMipLevels);
    const float3 specularIrradianceIBL = t_PrefilteredEnvironment.SampleLevel(s_Anisotropic8Clamp, view.ReflectionDir, surface.Roughness * numMipLevels).rgb;

    const float2 specularFactors = t_BRDFLUT.Sample(s_TrilinearClamp, float2(surface.Roughness, view.NoV)).rg;

    const float3 diffuseReflection = diffuseIrradianceIBL * surface.Albedo * (1.0 - surface.Metalness);
    const float3 specularReflection = specularIrradianceIBL * (surface.F0 * specularFactors.x + specularFactors.y);

    return diffuseReflection + specularReflection;
}

float3 PBR_RenderingEquation(in const View view, in const Surface surface, in const Light light)
{
    float3 diffuseComponent = light.SolidAngle * surface.Albedo *
        (1.0 - surface.Metalness) * (1.0 - FresnelSchlick(surface.F0, view.NoL)) * view.NoL / PI;
    if (!c_UseDiffuseReflectoins)
        diffuseComponent = float3(0.0, 0.0, 0.0);
    
    const float3 halfDir = normalize(view.ViewDir + light.SpecularLightDir);
    const float specularNoL = max(dot(surface.SurfaceNormal, light.SpecularLightDir), Epsilon);
    const float NoH = max(dot(surface.SurfaceNormal, halfDir), Epsilon);
    const float VoH = max(dot(view.ViewDir, halfDir), Epsilon);

    float3 specularComponent = min(1.0, NDF_GGX(NoH, surface.Roughness) * light.SolidAngle / (4.0 * view.NoV)) *
        ShlickGGX(view.NoV, specularNoL, surface.Roughness) * FresnelSchlick(surface.F0, VoH);
    if (!c_UseSpecularReflections)
        specularComponent = float3(0.0, 0.0, 0.0);

    return light.Radiance * (diffuseComponent + specularComponent);
}

float3 DirectionalLightContribution(in const View view, in const Surface surface, in const float3 worldPos)
{
    float3 directLighting = float3(0.0, 0.0, 0.0);
    
    View currentView = view;
    
    for (uint directionalLightIndex = 0; directionalLightIndex < c_DirectionalLightsCount; ++directionalLightIndex)
    {
        const DirectionalLight directionalLight = t_DirectionalLights[directionalLightIndex];
        
        const float3 lightDir = normalize(-directionalLight.Direction);
        
        currentView.NoL = max(dot(surface.SurfaceNormal, lightDir), Epsilon);
        
        Light light;
        light.SpecularLightDir = lightDir;
        light.Radiance = directionalLight.Radiance;
        light.SolidAngle = directionalLight.SolidAngle;

        float3 lightContribution = PBR_RenderingEquation(currentView, surface, light);

        if (c_UseDirectionalShadows)
            lightContribution *= VisibilityForDirectionalLight(directionalLightIndex, worldPos, surface.SurfaceNormal);

        directLighting += lightContribution;
    }
    
    return directLighting;
}

float3 PointLightContribution(in const View view, in const Surface surface, in const float3 worldPos)
{
    float3 directLighting = float3(0.0, 0.0, 0.0);
    
    View currentView = view;
    
    for (uint pointLightIndex = 0; pointLightIndex < c_PointLightsCount; ++pointLightIndex)
    {
        const PointLight pointLight = t_PointLights[pointLightIndex];
        
        const float3 lightWorldPos = pointLight.Position;
        
        const float sphereDist = max(length(lightWorldPos - worldPos), pointLight.Radius);
        
        const float sizeRelation = pointLight.Radius / sphereDist;
        const float solidAngle = 2.0 * PI * (1.0 - sqrt(1.0 - sizeRelation * sizeRelation));

        const float3 sphereDir = normalize(lightWorldPos - worldPos);
        const float sphereSin = pointLight.Radius / sphereDist;
        const float sphereCos = sqrt(1.0 - sphereSin * sphereSin);
        float3 lightDir = ApproximateClosestSphereDir(view.ReflectionDir, sphereCos, sphereDir * sphereDist, sphereDir, sphereDist, pointLight.Radius);
        
        float specularNoL = dot(surface.SurfaceNormal, lightDir);
        ClampDirToHorizon(lightDir, specularNoL, surface.SurfaceNormal, Epsilon);

        // Plane equation: Ax + By + Cz + D = 0, where (A, B, C) is its normal.
        const float geometryPlaneD = -dot(worldPos, surface.GeometryNormal);
        const float geometryPlaneH = dot(lightWorldPos, surface.GeometryNormal) + geometryPlaneD;
        const float geometryFalloff = saturate((geometryPlaneH + pointLight.Radius) / (2.0 * pointLight.Radius));
        
        const float surfacePlaneD = -dot(worldPos, surface.SurfaceNormal);
        const float surfacePlaneH = dot(lightWorldPos, surface.SurfaceNormal) + surfacePlaneD;
        const float surfaceFalloff = saturate((surfacePlaneH + pointLight.Radius) / (2.0 * pointLight.Radius));

        const float falloff = geometryFalloff * surfaceFalloff;
        
        currentView.NoL = max(dot(surface.SurfaceNormal, sphereDir), surfaceFalloff * sphereSin);
        
        Light light;
        light.SpecularLightDir = lightDir;
        light.Radiance = pointLight.Radiance;
        light.SolidAngle = solidAngle;

        float3 lightContribution = PBR_RenderingEquation(currentView, surface, light) * falloff;
        
        if (c_UseOmnidirectionalShadows)
            lightContribution *= VisibilityForPointLight(pointLightIndex, worldPos, surface.SurfaceNormal);

        directLighting += lightContribution;
    }
    
    return directLighting;
}

float3 SpotLightContribution(in const View view, in const Surface surface, in const float3 worldPos)
{
    float3 directLighting = float3(0.0, 0.0, 0.0);
    
    View currentView = view;
    
    for (uint spotLightIndex = 0; spotLightIndex < c_SpotLightsCount; ++spotLightIndex)
    {
        const SpotLight spotLight = t_SpotLights[spotLightIndex];

        const float3 lightWorldPos = spotLight.Position;
        const float3 lightWorldDir = spotLight.Direction;

        float3 lightDir = normalize(lightWorldPos - worldPos);

        const float angle = dot(lightDir, -lightWorldDir);
        if (angle > spotLight.OuterCutoffCos)
        {
            const float sphereDist = max(length(lightWorldPos - worldPos), spotLight.Radius);
            
            const float sizeRelation = spotLight.Radius / sphereDist;
            const float solidAngle = 2.0 * PI * (1.0 - sqrt(1.0 - sizeRelation * sizeRelation));

            const float spotLightFactor = spotLight.InnerCutoffCos - spotLight.OuterCutoffCos;
            const float intensity = saturate((angle - spotLight.OuterCutoffCos) / spotLightFactor);
            
            currentView.NoL = max(dot(surface.SurfaceNormal, lightDir), Epsilon);
            
            Light light;
            light.SpecularLightDir = lightDir;
            light.Radiance = spotLight.Radiance;
            light.SolidAngle = solidAngle;

            float3 lightContribution = PBR_RenderingEquation(currentView, surface, light) * intensity;

            if (c_UseSpotShadows)
                lightContribution *= VisibilityForSpotLight(spotLightIndex, worldPos, surface.SurfaceNormal);

            directLighting += lightContribution;
        }
    }
    
    return directLighting;
}

float3 CalculateDirectLighting(in const View view, in const Surface surface, in const float3 worldPos)
{
    float3 directLighting = float3(0.0, 0.0, 0.0);
    
    directLighting += DirectionalLightContribution(view, surface, worldPos);
    directLighting += PointLightContribution(view, surface, worldPos);
    directLighting += SpotLightContribution(view, surface, worldPos);
    
    return directLighting;
}

#endif