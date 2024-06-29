#include "Buffers.hlsli"
#include "Input.hlsli"
#include "Samplers.hlsli"

struct InstanceInput
{
    float _emptyInstance  : _empty;
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

    float4x4 meshToWorld = mul(c_MeshToModel, c_ModelToWorld[instInput.a_TransformIndex]);
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

float3 BlinnPhongResolveDirectionalLight(float3 albedo, float3 worldPos, float3 normal, float threshold)
{
    float3 reflectedColor = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < c_DirectionalLightsCount; ++i)
    {
        float3 lightDir = -normalize(mul(float4(c_DirectionalLights[i].Direction, 0.0), c_ModelToWorld[c_DirectionalLights[i].TransformIndex]).xyz);

        float3 diffuse = max(dot(normal, lightDir), threshold) * albedo;

        float3 halfDir = normalize(lightDir + normalize(c_CameraPosition.xyz - worldPos));
        float3 lightAboveSurfaceFactor = max(dot(normal, lightDir), threshold);
        float3 specular = pow(max(dot(normal, halfDir), threshold), 256.0) * lightAboveSurfaceFactor;

        reflectedColor += c_DirectionalLights[i].Luminance * (diffuse + specular);
    }

    return reflectedColor;
}

float3 BlinnPhongResolvePointLight(float3 albedo, float3 worldPos, float3 normal, float threshold)
{
    float3 reflectedColor = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < c_PointLightsCount; ++i)
    {
        const float3 lightWorldPos = mul(float4(c_PointLights[i].Position, 1.0), c_ModelToWorld[c_PointLights[i].TransformIndex]).xyz;
        const float3 lightDir = normalize(lightWorldPos - worldPos);

        const float3 diffuse = max(dot(normal, lightDir), threshold) * albedo;

        const float3 halfDir = normalize(lightDir + normalize(c_CameraPosition.xyz - worldPos));
        float3 lightAboveSurfaceFactor = max(dot(normal, lightDir), threshold);
        float3 specular = pow(max(dot(normal, halfDir), threshold), 256.0) * lightAboveSurfaceFactor;

        const float dist = length(lightWorldPos - worldPos);
        const float attenuation = 1.0 / (1.0 + c_PointLights[i].Linear * dist + c_PointLights[i].Quadratic * dist * dist);

        reflectedColor += c_PointLights[i].Luminance * (diffuse + specular) * attenuation;
    }

    return reflectedColor;
}

float3 BlinnPhongResolveSpotLight(float3 albedo, float3 worldPos, float3 normal, float threshold)
{
    float3 reflectedColor = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < c_SpotLightsCount; ++i)
    {
        const float3 spotLightWorldPos = mul(float4(c_SpotLights[i].Position, 1.0), c_ModelToWorld[c_SpotLights[i].TransformIndex]).xyz;
        const float3 spotLightWorldDir = normalize(mul(float4(c_SpotLights[i].Direction, 0.0), c_ModelToWorld[c_SpotLights[i].TransformIndex]).xyz);

        const float3 lightDir = normalize(spotLightWorldPos - worldPos);

        const float angle = dot(lightDir, -spotLightWorldDir);
        if (angle > c_SpotLights[i].OuterCutoffCos)
        {
            const float3 diffuse = max(dot(normal, lightDir), threshold) * albedo;

            const float3 halfDir = normalize(lightDir + normalize(c_CameraPosition.xyz - worldPos));
            float3 lightAboveSurfaceFactor = max(dot(normal, lightDir), threshold);
            float3 specular = pow(max(dot(normal, halfDir), threshold), 256.0) * lightAboveSurfaceFactor;

            const float dist = length(spotLightWorldPos - worldPos);
            const float attenuation = 1.0 / (1.0 + c_SpotLights[i].Linear * dist + c_SpotLights[i].Quadratic * dist * dist);

            const float spotLightFactor = c_SpotLights[i].InnerCutoffCos - c_SpotLights[i].OuterCutoffCos;
            const float intensity = clamp((angle - c_SpotLights[i].OuterCutoffCos) / spotLightFactor, 0.0, 1.0);

            reflectedColor += c_SpotLights[i].Luminance * (diffuse + specular) * attenuation * intensity;
        }
    }

    return reflectedColor;
}

Texture2D<float3> t_Albedo    : register(t0);
Texture2D<float2> t_Normal    : register(t1);
Texture2D<float>  t_Metallic  : register(t2);
Texture2D<float>  t_Roughness : register(t3);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    const float3 albedo = t_Albedo.Sample(s_ActiveSampler, psInput.v_TexCoords).xyz;
    const float2 normalBC5 = t_Normal.Sample(s_ActiveSampler, psInput.v_TexCoords).xy;
    
    const float3 normalTangentSpace = float3(normalBC5.x, normalBC5.y, sqrt(saturate(1.0 - dot(normalBC5.xy, normalBC5.yx))));
    float3 normal = normalize(mul(
        normalTangentSpace,
        psInput.v_TangentToWorld
    ));

    const float threshold = 0.0001;
    const float3 indirect = float3(0.05, 0.05, 0.05);
    float3 outputColor = albedo * indirect;
    outputColor += BlinnPhongResolveDirectionalLight(albedo, psInput.v_WorldPos, normal, threshold);
    outputColor += BlinnPhongResolvePointLight(albedo, psInput.v_WorldPos, normal, threshold);
    outputColor += BlinnPhongResolveSpotLight(albedo, psInput.v_WorldPos, normal, threshold);

    return float4(outputColor, 1.0);
}