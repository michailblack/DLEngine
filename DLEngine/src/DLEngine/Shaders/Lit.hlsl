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
    float4 v_Position  : SV_POSITION;
    float3 v_WorldPos  : WORLD_POS;
    float3 v_Normal    : NORMAL;
    float2 v_TexCoords : TEXCOORDS;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4x4 meshToWorld = mul(c_MeshToModel, c_ModelToWorld[instInput.a_TransformIndex]);
    float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), meshToWorld);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);

    float3 axisX = normalize(meshToWorld[0].xyz);
    float3 axizY = normalize(meshToWorld[1].xyz);
    float3 axizZ = normalize(meshToWorld[2].xyz);
    vsOutput.v_Normal = vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ;
    vsOutput.v_TexCoords = vsInput.a_TexCoords;

    return vsOutput;
}

float3 BlinnPhongResolveDirectionalLight(float3 color, float3 worldPos, float3 normal, float threshold)
{
    float3 resultColor = color;
    for (uint i = 0; i < c_DirectionalLightsCount; ++i)
    {
        float3 lightDir = -normalize(mul(float4(c_DirectionalLights[i].Direction, 0.0), c_ModelToWorld[c_DirectionalLights[i].TransformIndex]).xyz);

        float3 diffuse = max(dot(normal, lightDir), threshold) * c_DirectionalLights[i].Luminance * color;

        float3 halfDir = normalize(lightDir + (c_CameraPosition.xyz - worldPos));
        float3 specular = pow(max(dot(normal, halfDir), threshold), 256.0);

        resultColor += diffuse + specular;
    }

    return resultColor;
}

float3 BlinnPhongResolvePointLight(float3 color, float3 worldPos, float3 normal, float threshold)
{
    float3 resultColor = color;
    for (uint i = 0; i < c_PointLightsCount; ++i)
    {
        const float3 lightWorldPos = mul(float4(c_PointLights[i].Position, 1.0), c_ModelToWorld[c_PointLights[i].TransformIndex]).xyz;
        const float3 lightDir = normalize(lightWorldPos - worldPos);

        const float3 diffuse = max(dot(normal, lightDir), threshold) * c_PointLights[i].Luminance * color;

        const float3 halfDir = normalize(lightDir + (c_CameraPosition.xyz - worldPos));
        const float3 specular = pow(max(dot(normal, halfDir), threshold), 256.0);

        const float dist = length(lightWorldPos - worldPos);
        const float attenuation = 1.0 / (1.0 + c_PointLights[i].Linear * dist + c_PointLights[i].Quadratic * dist * dist);

        resultColor += (diffuse + specular) * attenuation;
    }

    return resultColor;
}

float3 BlinnPhongResolveSpotLight(float3 color, float3 worldPos, float3 normal, float threshold)
{
    float3 resultColor = color;
    for (uint i = 0; i < c_SpotLightsCount; ++i)
    {
        const float3 spotLightWorldPos = mul(float4(c_SpotLights[i].Position, 1.0), c_ModelToWorld[c_SpotLights[i].TransformIndex]).xyz;
        const float3 spotLightWorldDir = normalize(mul(float4(c_SpotLights[i].Direction, 0.0), c_ModelToWorld[c_SpotLights[i].TransformIndex]).xyz);

        const float3 lightDir = normalize(spotLightWorldPos - worldPos);

        const float angle = dot(lightDir, -spotLightWorldDir);
        if (angle > c_SpotLights[i].OuterCutoffCos)
        {
            const float3 diffuse = max(dot(normal, lightDir), threshold) * c_SpotLights[i].Luminance * color;

            const float3 halfDir = normalize(lightDir + (c_CameraPosition.xyz - worldPos));
            const float3 specular = pow(max(dot(normal, halfDir), threshold), 256.0);

            const float dist = length(spotLightWorldPos - worldPos);
            const float attenuation = 1.0 / (1.0 + c_SpotLights[i].Linear * dist + c_SpotLights[i].Quadratic * dist * dist);

            const float spotLightFactor = c_SpotLights[i].InnerCutoffCos - c_SpotLights[i].OuterCutoffCos;
            const float intensity = clamp((angle - c_SpotLights[i].OuterCutoffCos) / spotLightFactor, 0.0, 1.0);

            resultColor += (diffuse + specular) * attenuation * intensity;
        }
    }

    return resultColor;
}

Texture2D<float3> g_Albedo : register(t0);

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    float3 albedo = g_Albedo.Sample(g_AnisotropicClamp, psInput.v_TexCoords).xyz;
    const float threshold = 0.0001;
    float3 ambient = albedo * 0.05;
    float3 color = BlinnPhongResolveDirectionalLight(ambient, psInput.v_WorldPos, psInput.v_Normal, threshold);
    color = BlinnPhongResolvePointLight(color, psInput.v_WorldPos, psInput.v_Normal, threshold);
    color = BlinnPhongResolveSpotLight(color, psInput.v_WorldPos, psInput.v_Normal, threshold);
    return float4(color, 1.0);
}