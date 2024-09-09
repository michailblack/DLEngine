#include "Include/Lighting.hlsli"

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
    float4x4 a_Transform           : TRANSFORM;
    float    a_DissolutionDuration : DISSOLUTION_DURATION;
    float    a_ElapsedTime         : ELAPSED_TIME;
};

struct VertexOutput
{
    float4   v_Position       : SV_POSITION;
    float3   v_WorldPos       : WORLD_POS;
    float3   v_Normal         : NORMAL;
    float2   v_TexCoords      : TEXCOORDS;
    float3x3 v_TangentToWorld : TANGENT_TO_WORLD;
    float    v_Dissolution    : DISSOLUTION;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), instInput.a_Transform);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);

    vsOutput.v_TexCoords = vsInput.a_TexCoords;
    
    const float3x3 normalMatrix = ConstructNormalMatrix(instInput.a_Transform);
    
    vsOutput.v_Normal = mul(vsInput.a_Normal, normalMatrix);
    
    const float3 T = mul(vsInput.a_Tangent, normalMatrix);
    const float3 B = mul(vsInput.a_Bitangent, normalMatrix);
    
    vsOutput.v_TangentToWorld = float3x3(T, B, vsOutput.v_Normal);
    
    vsOutput.v_Dissolution = saturate(instInput.a_ElapsedTime / instInput.a_DissolutionDuration);

    return vsOutput;
}

Texture2D<float> t_DissolutionNoiseMap : register(t16);

static const float3 HighlightColor = float3(128.0, 128.0, 0.0);

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
    
    const float dissolutionNoise = t_DissolutionNoiseMap.Sample(s_NearestWrap, psInput.v_TexCoords).r;

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

    float3 indirectLighting = IBL(view, surface);
    if (!c_UseIBL)
        indirectLighting = c_IndirectLightingRadiance * albedo;

    const float3 directLighting = CalculateDirectLighting(view, surface, psInput.v_WorldPos);
    
    float3 outputColor = indirectLighting + directLighting;
    
    const float3 highlightColor = outputColor * HighlightColor;
    
    const float falloff = 0.15 / Epsilon;
    const float dissolutionAlpha = saturate((psInput.v_Dissolution - dissolutionNoise) / max(fwidth(psInput.v_Dissolution), Epsilon) / falloff);
    
    outputColor = lerp(highlightColor, outputColor, dissolutionAlpha);
    
    return float4(outputColor, dissolutionAlpha);
}