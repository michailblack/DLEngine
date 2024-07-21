#include "Buffers.hlsli"
#include "Input.hlsli"
#include "Samplers.hlsli"

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

static const float PI = 3.14159265359;

float GGX(float NdotH, float roughness)
{
    const float r = roughness * roughness;

    float denom = NdotH * NdotH * (r * r - 1.0) + 1.0;
    denom = PI * denom * denom;

    return r * r / denom;
}

float GGX_Smith(float NdotV, float NdotL, float roughness)
{
    const float r = roughness * roughness;

    return 2.0 / (sqrt(1.0 + (r * r * (1.0 - NdotV) / NdotV)) * sqrt(1.0 + (r * r * (1.0 - NdotL) / NdotL)));
}

float3 FresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float3 RenderingEquation(
    float3 normal, float3 viewDir, float3 lightDir,
    float3 albedo, float metallic, float roughness, float solidAngle,
    float3 lightLuminance
)
{
    const float3 halfDir = normalize(viewDir + lightDir);

    const float diffuseCosTheta = dot(normal, lightDir);
    const float specularCosTheta = dot(halfDir, lightDir);

    const float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    const float threshold = 0.0001;
    const float NdotL = max(dot(normal, lightDir), threshold);
    const float NdotV = max(dot(normal, viewDir), threshold);
    const float NdotH = max(dot(normal, halfDir), threshold);

    return lightLuminance * (
        1.0 / PI * solidAngle * albedo * (1.0 - metallic) * (1.0 - FresnelSchlick(F0, diffuseCosTheta)) * NdotL +
        min(1.0, (GGX(NdotH, roughness) * solidAngle) / (4.0 * NdotV)) *
        GGX_Smith(NdotV, NdotL, roughness) * FresnelSchlick(F0, specularCosTheta)
    );
}

Texture2D<float3> t_Albedo    : register(t5);
Texture2D<float2> t_Normal    : register(t6);
Texture2D<float>  t_Metallic  : register(t7);
Texture2D<float>  t_Roughness : register(t8);

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

    const float3 indirect = float3(0.05, 0.05, 0.05);

    float3 outputColor = albedo * indirect;

    const float3 viewDir = normalize(c_CameraPosition - psInput.v_WorldPos);

    uint stride = 0;
    uint lightsCount = 0;

    t_DirectionalLights.GetDimensions(lightsCount, stride);

    uint i = 0;
    for (; i < lightsCount; ++i)
    {
        const float3 lightDir = -normalize(mul(float4(t_DirectionalLights[i].Direction, 0.0), t_ModelToWorld[t_DirectionalLights[i].TransformIndex]).xyz);

        outputColor += RenderingEquation(
            surfaceNormal, viewDir, lightDir,
            albedo, metallic, roughness, t_DirectionalLights[i].SolidAngle,
            t_DirectionalLights[i].Radiance
        );
    }

    t_PointLights.GetDimensions(lightsCount, stride);

    i = 0;
    for (; i < lightsCount; ++i)
    {
        const float3 lightWorldPos = mul(float4(t_PointLights[i].Position, 1.0), t_ModelToWorld[t_PointLights[i].TransformIndex]).xyz;
        const float3 lightDir = normalize(lightWorldPos - psInput.v_WorldPos);

        const float sizeRelation = t_PointLights[i].Radius / length(lightWorldPos - psInput.v_WorldPos);
        const float solidAngle = 2.0 * PI * (1.0 - sqrt(1.0 - sizeRelation * sizeRelation));

        outputColor += RenderingEquation(
            surfaceNormal, viewDir, lightDir,
            albedo, metallic, roughness, solidAngle,
            t_PointLights[i].Radiance
        );
    }

    t_SpotLights.GetDimensions(lightsCount, stride);

    i = 0;
    for (; i < lightsCount; ++i)
    {
        const float3 spotLightWorldPos = mul(float4(t_SpotLights[i].Position, 1.0), t_ModelToWorld[t_SpotLights[i].TransformIndex]).xyz;
        const float3 spotLightWorldDir = normalize(mul(float4(t_SpotLights[i].Direction, 0.0), t_ModelToWorld[t_SpotLights[i].TransformIndex]).xyz);

        const float3 lightDir = normalize(spotLightWorldPos - psInput.v_WorldPos);

        const float angle = dot(lightDir, -spotLightWorldDir);
        if (angle > t_SpotLights[i].OuterCutoffCos)
        {
            const float sizeRelation = t_SpotLights[i].Radius / length(spotLightWorldPos - psInput.v_WorldPos);
            const float solidAngle = 2.0 * PI * (1.0 - sqrt(1.0 - sizeRelation * sizeRelation));

            const float spotLightFactor = t_SpotLights[i].InnerCutoffCos - t_SpotLights[i].OuterCutoffCos;
            const float intensity = saturate((angle - t_SpotLights[i].OuterCutoffCos) / spotLightFactor);

            outputColor += RenderingEquation(
                surfaceNormal, viewDir, lightDir,
                albedo, metallic, roughness, solidAngle,
                t_SpotLights[i].Radiance
            ) * intensity;
        }
    }

    return float4(outputColor, 1.0);
}