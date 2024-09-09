#include "Buffers.hlsli"
#include "Input.hlsli"
#include "Samplers.hlsli"
#include "BRDF.hlsli"

struct InstanceInput
{
    uint a_TransformIndex : TRANSFORM_INDEX;
};

struct VertexOutput
{
    float4   v_Position       : SV_POSITION;
    float3   v_WorldPos       : WORLD_POS;
    float3   v_Normal         : NORMAL;
    float2   v_TexCoords      : TEXCOORDS;
    float3x3 v_TangentToWorld : TANGENT_TO_WORLD;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4x4 meshToWorld = mul(c_MeshToModel, t_ModelToWorld[instInput.a_TransformIndex]);
    float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), meshToWorld);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);

    vsOutput.v_TexCoords = vsInput.a_TexCoords;

    float3 axisX = normalize(meshToWorld[0].xyz);
    float3 axizY = normalize(meshToWorld[1].xyz);
    float3 axizZ = normalize(meshToWorld[2].xyz);
    vsOutput.v_Normal = normalize(vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ);

    float3 T = normalize(vsInput.a_Tangent.x * axisX + vsInput.a_Tangent.y * axizY + vsInput.a_Tangent.z * axizZ);
    float3 B = normalize(vsInput.a_Bitangent.x * axisX + vsInput.a_Bitangent.y * axizY + vsInput.a_Bitangent.z * axizZ);

    vsOutput.v_TangentToWorld = float3x3(T, B, vsOutput.v_Normal);

    return vsOutput;
}

struct View
{
    float3 ViewDir;
    float3 ReflectionDir;
    float3 HalfDir;
    float NoV;
    float NoL;
    float NoH;
};

struct Surface
{
    float3 Albedo;
    float3 GeometryNormal;
    float3 SurfaceNormal;
    float Metallic;
    float Rough;
    float Rough2;
    float Rough4;
    float3 F0;
};

struct Light
{
    float3 LightDir;
    float3 Radiance;
    float SolidAngle;
};

float3 CalculateEnvironmentReflection(in const View view, in const Surface surface)
{
    const float3 diffuseIrradianceIBL = t_DiffuseIrradianceIBL.Sample(s_AnisotropicWrap, surface.SurfaceNormal).rgb;

    float width, height, numMipLevels;
    t_SpecularIrradianceIBL.GetDimensions(0, width, height, numMipLevels);
    const float3 specularIrradianceIBL = t_SpecularIrradianceIBL.SampleLevel(s_AnisotropicWrap, view.ReflectionDir, surface.Rough * numMipLevels).rgb;

    const float2 specularFactors = t_SpecularFactorIBL.Sample(s_TrilinearClamp, float2(surface.Rough, view.NoV)).rg;

    const float3 diffuseReflection = diffuseIrradianceIBL * surface.Albedo * (1.0 - surface.Metallic);
    const float3 specularReflection = specularIrradianceIBL * (surface.F0 * specularFactors.x + specularFactors.y);

    return diffuseReflection + specularReflection;
}

float3 RenderingEquation(in const View view, in const Surface surface, in const Light light)
{
    const float diffuseCosTheta = dot(surface.SurfaceNormal, light.LightDir);
    const float specularCosTheta = dot(view.HalfDir, light.LightDir);

    float3 diffuseComponent = light.SolidAngle * surface.Albedo * (1.0 - surface.Metallic) * (1.0 - FresnelSchlick(surface.F0, diffuseCosTheta)) * view.NoL / PI;
    if (!c_RendererSettings.DiffuseReflectoinsEnabled)
        diffuseComponent = float3(0.0, 0.0, 0.0);

    float3 specularComponent = min(1.0, (TrowbridgeReitzNDF(view.NoH, surface.Rough4) * light.SolidAngle) / (4.0 * view.NoV)) *
        SmithGAF(view.NoV, view.NoL, surface.Rough4) * FresnelSchlick(surface.F0, specularCosTheta);
    if (!c_RendererSettings.SpecularReflectionsEnabled)
        specularComponent = float3(0.0, 0.0, 0.0);

    return light.Radiance * (diffuseComponent + specularComponent);
}

// May return direction pointing beneath surface horizon (dot(N, dir) < 0), use clampDirToHorizon to fix it.
// sphereCos is cosine of the light source angular halfsize (2D angle, not solid angle).
// sphereRelPos is position of a sphere relative to surface:
// 'sphereDir == normalize(sphereRelPos)' and 'sphereDir * sphereDist == sphereRelPos'
float3 ApproximateClosestSphereDir(float3 reflectionDir, float sphereCos,
    float3 sphereRelPos, float3 sphereDir, float sphereDist, float sphereRadius)
{
    float3 closestSphereDir = float3(0.0, 0.0, 0.0);
    const float RoS = dot(reflectionDir, sphereDir);

    const bool intersects = RoS >= sphereCos;
    if (intersects)
    {
        closestSphereDir = reflectionDir;
    }
    else if (RoS < 0.0)
    {
        closestSphereDir = sphereDir;
    }
    else
    {
        const float3 closestPointDir = normalize(reflectionDir * sphereDist * RoS - sphereRelPos);
        closestSphereDir = normalize(sphereRelPos + sphereRadius * closestPointDir);
    }

    return closestSphereDir;
}

// Input dir and NoD is N and NoL in a case of lighting computation 
void ClampDirToHorizon(inout float3 dir, inout float NoD, float3 normal, float minNoD)
{
    if (NoD < minNoD)
    {
        dir = normalize(dir + (minNoD - NoD) * normal);
        NoD = minNoD;
    }
}

Texture2D<float3> t_Albedo    : register(t8);
Texture2D<float2> t_Normal    : register(t9);
Texture2D<float>  t_Metallic  : register(t10);
Texture2D<float>  t_Roughness : register(t11);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    const float3 albedo    = t_Albedo.Sample(s_ActiveSampler, psInput.v_TexCoords).rgb;
    const float  metallic  = t_Metallic.Sample(s_ActiveSampler, psInput.v_TexCoords).r;
    const float  roughness = t_Roughness.Sample(s_ActiveSampler, psInput.v_TexCoords).r;
    const float2 normalBC5 = t_Normal.Sample(s_ActiveSampler, psInput.v_TexCoords).rg;
    
    const float3 normalTangentSpace = float3(normalBC5.x, normalBC5.y, sqrt(saturate(1.0 - dot(normalBC5.xy, normalBC5.yx))));
    const float3 surfaceNormal = normalize(mul(
        normalTangentSpace,
        psInput.v_TangentToWorld
    ));

    float rough = roughness;
    if (c_RendererSettings.OverwriteRoughness)
        rough = c_RendererSettings.OverwrittenRoughness;

    Surface surface;
    surface.Albedo = albedo;
    surface.GeometryNormal = psInput.v_Normal;
    surface.SurfaceNormal = surfaceNormal;
    surface.Metallic = metallic;
    surface.Rough = rough;
    surface.Rough2 = rough * rough;
    surface.Rough4 = rough * rough * rough * rough;
    surface.F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    View view;
    view.ViewDir = normalize(c_CameraPosition - psInput.v_WorldPos);
    view.ReflectionDir = normalize(reflect(-view.ViewDir, surface.SurfaceNormal));
    view.NoV = max(dot(view.ViewDir, surface.SurfaceNormal), THRESHOLD);
    view.HalfDir = float3(0.0, 0.0, 0.0);
    view.NoL = -1.0;
    view.NoH = -1.0;

    float3 indirectLighting = CalculateEnvironmentReflection(view, surface);
    if (!c_RendererSettings.UseIBL)
        indirectLighting = c_RendererSettings.IndirectLightingRadiance * albedo;

    float3 outputColor = indirectLighting;

    uint stride = 0;
    uint lightsCount = 0;
    t_DirectionalLights.GetDimensions(lightsCount, stride);

    uint i = 0;
    for (; i < lightsCount; ++i)
    {
        const DirectionalLight directionalLight = t_DirectionalLights[i];

        Light light;
        light.LightDir = -normalize(mul(float4(directionalLight.Direction, 0.0), t_ModelToWorld[directionalLight.TransformIndex]).xyz);
        light.Radiance = directionalLight.Radiance;
        light.SolidAngle = directionalLight.SolidAngle;

        view.HalfDir = normalize(view.ViewDir + light.LightDir);
        view.NoL = max(dot(surface.SurfaceNormal, light.LightDir), THRESHOLD);
        view.NoH = max(dot(surface.SurfaceNormal, view.HalfDir), THRESHOLD);

        outputColor += RenderingEquation(view, surface, light);
    }

    t_PointLights.GetDimensions(lightsCount, stride);

    i = 0;
    for (; i < lightsCount; ++i)
    {
        const PointLight pointLight = t_PointLights[i];

        const float3 lightWorldPos = mul(float4(pointLight.Position, 1.0), t_ModelToWorld[pointLight.TransformIndex]).xyz;

        const float sizeRelation = pointLight.Radius / length(lightWorldPos - psInput.v_WorldPos);
        const float solidAngle = 2.0 * PI * (1.0 - sqrt(1.0 - sizeRelation * sizeRelation));

        const float sphereDist = length(lightWorldPos - psInput.v_WorldPos);
        const float3 sphereDir = normalize(lightWorldPos - psInput.v_WorldPos);
        float sphereCos = sqrt(sphereDist * sphereDist - pointLight.Radius * pointLight.Radius) / sphereDist;

        float3 lightDir = ApproximateClosestSphereDir(view.ReflectionDir, sphereCos, sphereDir * sphereDist, sphereDir, sphereDist, pointLight.Radius);
        float NoD = dot(surface.SurfaceNormal, lightDir);
        ClampDirToHorizon(lightDir, NoD, surface.SurfaceNormal, THRESHOLD);

        Light light;
        light.LightDir = lightDir;
        light.Radiance = pointLight.Radiance;
        light.SolidAngle = solidAngle;

        view.HalfDir = normalize(view.ViewDir + light.LightDir);
        view.NoL = max(dot(surface.SurfaceNormal, light.LightDir), THRESHOLD);
        view.NoH = max(dot(surface.SurfaceNormal, view.HalfDir), THRESHOLD);

        // Plane equation: Ax + By + Cz + D = 0, where (A, B, C) is its normal.
        const float geometryPlaneD = -dot(psInput.v_WorldPos, surface.GeometryNormal);
        const float geometryPlaneH = dot(lightWorldPos, surface.GeometryNormal) + geometryPlaneD;
        const float geometryFalloff = saturate((geometryPlaneH + pointLight.Radius) / (2.0 * pointLight.Radius));

        const float surfacePlaneD = -dot(psInput.v_WorldPos, surface.SurfaceNormal);
        const float surfacePlaneH = dot(lightWorldPos, surface.SurfaceNormal) + surfacePlaneD;
        const float surfaceFalloff = saturate((surfacePlaneH + pointLight.Radius) / (2.0 * pointLight.Radius));

        outputColor += RenderingEquation(view, surface, light) * geometryFalloff * surfaceFalloff;
    }

    t_SpotLights.GetDimensions(lightsCount, stride);

    i = 0;
    for (; i < lightsCount; ++i)
    {
        const SpotLight spotLight = t_SpotLights[i];

        const float3 lightWorldPos = mul(float4(spotLight.Position, 1.0), t_ModelToWorld[spotLight.TransformIndex]).xyz;
        const float3 lightWorldDir = normalize(mul(float4(spotLight.Direction, 0.0), t_ModelToWorld[spotLight.TransformIndex]).xyz);

        Light light;
        light.LightDir = normalize(lightWorldPos - psInput.v_WorldPos);

        const float angle = dot(light.LightDir, -lightWorldDir);
        if (angle > spotLight.OuterCutoffCos)
        {
            const float sizeRelation = spotLight.Radius / length(lightWorldPos - psInput.v_WorldPos);
            const float solidAngle = 2.0 * PI * (1.0 - sqrt(1.0 - sizeRelation * sizeRelation));

            light.Radiance = spotLight.Radiance;
            light.SolidAngle = solidAngle;

            view.HalfDir = normalize(view.ViewDir + light.LightDir);
            view.NoL = max(dot(surface.SurfaceNormal, light.LightDir), THRESHOLD);
            view.NoH = max(dot(surface.SurfaceNormal, view.HalfDir), THRESHOLD);

            const float spotLightFactor = spotLight.InnerCutoffCos - spotLight.OuterCutoffCos;
            const float intensity = saturate((angle - spotLight.OuterCutoffCos) / spotLightFactor);

            outputColor += RenderingEquation(view, surface, light) * intensity;
        }
    }

    return float4(outputColor, 1.0);
}