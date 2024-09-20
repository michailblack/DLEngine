#include "Include/Buffers.hlsli"
#include "Include/Common.hlsli"
#include "Include/Samplers.hlsli"

struct VertexInput
{
    float3 a_Position  : POSITION;
    float3 a_Normal    : NORMAL;
    float3 a_Tangent   : TANGENT;
    float3 a_Bitangent : BITANGENT;
    float2 a_TexCoords : TEXCOORDS;
};

struct TransformInput
{
    float4x4 a_DecalToWorld : DECAL_TO_WORLD;
    float4x4 a_WorldToDecal : WORLD_TO_DECAL;
};

struct InstanceInput
{
    float3 a_TintColor          : DECAL_TINT_COLOR;
    uint2  a_ParentInstanceUUID : PARENT_INSTANCE_UUID;
};

struct VertexOutput
{
    float4                 v_Position           : SV_POSITION;
    float4x4               v_DecalToWorld       : DECAL_TO_WORLD;
    float4x4               v_WorldToDecal       : WORLD_TO_DECAL;
    nointerpolation float3 v_TintColor          : TINT_COLOR;
    nointerpolation uint2  v_ParentInstanceUUID : PARENT_INSTANCE_UUID;
};

VertexOutput mainVS(VertexInput vsInput, TransformInput transformInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), transformInput.a_DecalToWorld);

    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);
    vsOutput.v_DecalToWorld = transformInput.a_DecalToWorld;
    vsOutput.v_WorldToDecal = transformInput.a_WorldToDecal;
    vsOutput.v_TintColor = instInput.a_TintColor;
    vsOutput.v_ParentInstanceUUID = instInput.a_ParentInstanceUUID;
    
    return vsOutput;
}

struct PixelOutput
{
    float4 o_Albedo                 : SV_TARGET0;
    float4 o_MetalnessRoughness     : SV_TARGET1;
    float4 o_GeometrySurfaceNormals : SV_TARGET2;
    float4 o_Emission               : SV_TARGET3;
};

Texture2D<float4> t_DecalNormalAlpha               : register(t20);

Texture2D<float4> t_GBuffer_GeometrySurfaceNormals : register(t21);
Texture2D<uint2>  t_GBuffer_InstanceUUID           : register(t22);
Texture2D<float>  t_GBuffer_Depth                  : register(t23);

static const float NormalThresholdCos = cos(radians(150));
static const float DecalRoughness     = 0.2;

PixelOutput mainPS(VertexOutput psInput)
{
    const uint2 instanceUUID = t_GBuffer_InstanceUUID.Load(int3(psInput.v_Position.xy, 0)).rg;
    if (!all(instanceUUID == psInput.v_ParentInstanceUUID))
        discard;
    
    const float2 uv = psInput.v_Position.xy * float2(c_InvViewportWidth, c_InvViewportHeight);
    
    const float ndcX = uv.x * 2.0 - 1.0;
    const float ndcY = uv.y * -2.0 + 1.0;
    const float depth = t_GBuffer_Depth.Sample(s_NearestClamp, uv).r;
    
    float4 worldPos = mul(float4(ndcX, ndcY, depth, 1.0), c_InvViewProjection);
    worldPos /= worldPos.w;
    
    const float3 decalSpacePos = mul(float4(worldPos.xyz, 1.0), psInput.v_WorldToDecal).xyz;
    
    // Discard pixels outside of the unit cube
    clip(float3(0.5, 0.5, 0.5) - abs(decalSpacePos.xyz));
    
    // Sample decal normal and alpha
    float2 decalUV = decalSpacePos.xy + 0.5;
    decalUV.y = 1.0 - decalUV.y;
    const float4 decalNormalAlpha = t_DecalNormalAlpha.Sample(s_TrilinearClamp, decalUV);
    const float3 decalNormalTangentSpace = normalize(decalNormalAlpha.rgb * 2.0 - 1.0);
    const float decalAlpha = decalNormalAlpha.a;

    // Build TBM matrix
    const float4 geometrySurfaceNormals = t_GBuffer_GeometrySurfaceNormals.Sample(s_NearestClamp, uv);
    const float3 surfaceNormal = unpackOctahedron(geometrySurfaceNormals.zw);
    
    const float3 decalForward = normalize(psInput.v_DecalToWorld[2].xyz);
    clip(dot(-decalForward, surfaceNormal) - NormalThresholdCos);
    
    const float3 decalRight = normalize(psInput.v_DecalToWorld[0].xyz);
    const float3 T = normalize(decalRight - dot(decalRight, surfaceNormal) * surfaceNormal);
    const float3 B = normalize(cross(T, surfaceNormal));
    const float3x3 TBN = float3x3(T, B, surfaceNormal);
    
    // Transform decal normal to world space
    const float3 decalNormalWorldSpace = mul(decalNormalTangentSpace, TBN);
    
    const float3 interpolatedSurfaceNormal = normalize(lerp(surfaceNormal, decalNormalWorldSpace, decalAlpha));
    
    PixelOutput psOutput;
    psOutput.o_Albedo = float4(psInput.v_TintColor, decalAlpha);
    psOutput.o_MetalnessRoughness = float4(0.0, DecalRoughness, 0.0, decalAlpha);
    psOutput.o_GeometrySurfaceNormals = float4(geometrySurfaceNormals.rg, packOctahedron(interpolatedSurfaceNormal));
    psOutput.o_Emission = float4(0.0, 0.0, 0.0, decalAlpha);
    
    return psOutput;
}