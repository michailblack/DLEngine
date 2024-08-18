#include "Include/Buffers.hlsli"
#include "Include/Common.hlsli"
#include "Include/PBR.hlsli"
#include "Include/PBR_Resources.hlsli"
#include "Include/Samplers.hlsli"

struct VertexInput
{
    float3 a_Position  : POSITION;
    float3 a_Normal    : NORMAL;
    float3 a_Tangent   : TANGENT;
    float3 a_Bitangent : BITANGENT;  
    float2 a_TexCoords : TEXCOORDS;
};


struct InstanceInput
{
    float4x4 a_Transform : TRANSFORM;
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

    const float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), instInput.a_Transform);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);

    vsOutput.v_TexCoords = vsInput.a_TexCoords;

    float3 axisX = normalize(instInput.a_Transform[0].xyz);
    float3 axizY = normalize(instInput.a_Transform[1].xyz);
    float3 axizZ = normalize(instInput.a_Transform[2].xyz);
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
    float NoV;
    float NoL;
    float NoH;
    float VoL;
    float VoH;
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

float3 RenderingEquation(View view, Surface surface, float3 lightRadiance, float lightSolidAngle)
{
    float3 diffuseComponent = lightSolidAngle * surface.Albedo *
        (1.0 - surface.Metalness) * (1.0 - FresnelSchlick(surface.F0, view.NoL)) * view.NoL / PI;
    if (!c_UseDiffuseReflectoins)
        diffuseComponent = float3(0.0, 0.0, 0.0);

    float3 specularComponent = min(1.0, NDF_GGX(view.NoH, surface.Roughness) * lightSolidAngle / (4.0 * view.NoV)) *
        ShlickGGX(view.NoV, view.NoL, surface.Roughness) * FresnelSchlick(surface.F0, view.VoH);
    if (!c_UseSpecularReflections)
        specularComponent = float3(0.0, 0.0, 0.0);

    return lightRadiance * (diffuseComponent + specularComponent);
}

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    const float3 albedo = t_Albedo.Sample(s_ActiveSampler, psInput.v_TexCoords).rgb;
    
    float metalness = c_DefaultMetalness;
    if (c_HasMetalnessMap)
        metalness = t_Metalness.Sample(s_ActiveSampler, psInput.v_TexCoords).r;

    float roughness = c_DefaultRoughness;
    if (c_HasRoughnessMap)
        roughness = t_Roughness.Sample(s_ActiveSampler, psInput.v_TexCoords).r;    

    float3 surfaceNormal = psInput.v_Normal;
    if (c_UseNormalMap)
    {
        float2 normalMapSampleCoords = psInput.v_TexCoords;
        if (c_FlipNormalMapY)
            normalMapSampleCoords.y = 1.0 - normalMapSampleCoords.y;
        const float2 normalBC5 = t_Normal.Sample(s_ActiveSampler, normalMapSampleCoords).rg;
    
        const float3 normalTangentSpace = float3(normalBC5.x, normalBC5.y, sqrt(saturate(1.0 - dot(normalBC5.xy, normalBC5.yx))));
        surfaceNormal = normalize(mul(
            normalTangentSpace,
            psInput.v_TangentToWorld
        ));
    }

    float rough = roughness;
    if (c_OverwriteRoughness)
        rough = c_OverwrittenRoughness;

    rough = max(rough, 0.01); // To keep specular highlight for small values

    Surface surface;
    surface.Albedo = albedo;
    surface.GeometryNormal = psInput.v_Normal;
    surface.SurfaceNormal = surfaceNormal;
    surface.Metalness = metalness;
    surface.Roughness = rough;
    surface.F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);

    View view;
    view.ViewDir = normalize(c_CameraPosition - psInput.v_WorldPos);
    view.ReflectionDir = normalize(reflect(-view.ViewDir, surface.SurfaceNormal));
    view.NoV = max(dot(surface.SurfaceNormal, view.ViewDir), Epsilon);
    view.NoL = -1.0;
    view.VoL = -1.0;
    view.NoH = -1.0;
    view.VoH = -1.0;

    float3 indirectLighting = IBL(view, surface);
    if (!c_UseIBL)
        indirectLighting = c_IndirectLightingRadiance * albedo;

    float3 outputColor = indirectLighting;

    uint stride = 0;
    uint lightsCount = 0;

    t_DirectionalLights.GetDimensions(lightsCount, stride);
    uint i = 0;
    for (; i < lightsCount; ++i)
    {
        const DirectionalLight directionalLight = t_DirectionalLights[i];

        const float3 lightDir = normalize(-directionalLight.Direction);
        const float3 halfDir = normalize(view.ViewDir + lightDir);

        view.NoL = max(dot(surface.SurfaceNormal, lightDir), Epsilon);
        view.NoH = max(dot(surface.SurfaceNormal, halfDir), Epsilon);
        view.VoL = max(dot(view.ViewDir, lightDir), Epsilon);
        view.VoH = max(dot(view.ViewDir, halfDir), Epsilon);

        outputColor += RenderingEquation(view, surface, directionalLight.Radiance, directionalLight.SolidAngle);
    }

    t_PointLights.GetDimensions(lightsCount, stride);
    i = 0;
    for (; i < lightsCount; ++i)
    {
        const PointLight pointLight = t_PointLights[i];

        const float3 lightWorldPos = pointLight.Position;

        const float sizeRelation = pointLight.Radius / length(lightWorldPos - psInput.v_WorldPos);
        const float solidAngle = 2.0 * PI * (1.0 - sqrt(1.0 - saturate(sizeRelation * sizeRelation)));

        const float sphereDist = length(lightWorldPos - psInput.v_WorldPos);
        const float3 sphereDir = normalize(lightWorldPos - psInput.v_WorldPos);
        const float sphereSin = pointLight.Radius / sphereDist;
        const float sphereCos = sqrt(1.0 - sphereSin * sphereSin);
        float3 lightDir = ApproximateClosestSphereDir(view.ReflectionDir, sphereCos, sphereDir * sphereDist, sphereDir, sphereDist, pointLight.Radius);
        
        view.NoL = dot(surface.SurfaceNormal, lightDir);
        ClampDirToHorizon(lightDir, view.NoL, surface.SurfaceNormal, Epsilon);

        const float3 halfDir = normalize(view.ViewDir + lightDir);
        view.NoH = max(dot(surface.SurfaceNormal, halfDir), Epsilon);
        view.VoL = max(dot(view.ViewDir, lightDir), Epsilon);
        view.VoH = max(dot(view.ViewDir, halfDir), Epsilon);

        // Plane equation: Ax + By + Cz + D = 0, where (A, B, C) is its normal.
        const float geometryPlaneD = -dot(psInput.v_WorldPos, surface.GeometryNormal);
        const float geometryPlaneH = dot(lightWorldPos, surface.GeometryNormal) + geometryPlaneD;
        const float geometryFalloff = saturate((geometryPlaneH + pointLight.Radius) / (2.0 * pointLight.Radius));
        
        const float surfacePlaneD = -dot(psInput.v_WorldPos, surface.SurfaceNormal);
        const float surfacePlaneH = dot(lightWorldPos, surface.SurfaceNormal) + surfacePlaneD;
        const float surfaceFalloff = saturate((surfacePlaneH + pointLight.Radius) / (2.0 * pointLight.Radius));

        outputColor += RenderingEquation(view, surface, pointLight.Radiance, solidAngle) * geometryFalloff * surfaceFalloff;
    }

    t_SpotLights.GetDimensions(lightsCount, stride);
    i = 0;
    for (; i < lightsCount; ++i)
    {
        const SpotLight spotLight = t_SpotLights[i];

        const float3 lightWorldPos = spotLight.Position;
        const float3 lightWorldDir = spotLight.Direction;

        float3 lightDir = normalize(lightWorldPos - psInput.v_WorldPos);

        const float angle = dot(lightDir, -lightWorldDir);
        if (angle > spotLight.OuterCutoffCos)
        {
            const float sizeRelation = spotLight.Radius / length(lightWorldPos - psInput.v_WorldPos);
            const float solidAngle = 2.0 * PI * (1.0 - sqrt(1.0 - sizeRelation * sizeRelation));

            float3 halfDir = normalize(view.ViewDir + lightDir);
            view.NoL = max(dot(surface.SurfaceNormal, lightDir), Epsilon);
            view.NoH = max(dot(surface.SurfaceNormal, halfDir), Epsilon);
            view.VoL = max(dot(view.ViewDir, lightDir), Epsilon);
            view.VoH = max(dot(view.ViewDir, halfDir), Epsilon);

            const float spotLightFactor = spotLight.InnerCutoffCos - spotLight.OuterCutoffCos;
            const float intensity = saturate((angle - spotLight.OuterCutoffCos) / spotLightFactor);

            outputColor += RenderingEquation(view, surface, spotLight.Radiance, solidAngle) * intensity;
        }
    }

    return float4(outputColor, 1.0);
}