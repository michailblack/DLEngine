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
    float4x4 a_Transform : TRANSFORM;
};

struct InstanceInput
{
    float3 a_ParticleEmission        : INCINERATION_PARTICLE_EMISSION;
    float3 a_SpherePositionMeshSpace : INCINERATION_SPHERE_POSITION_MESH_SPACE;
    uint2  a_InstanceUUID            : INSTANCE_UUID;
    float  a_MaxSphereRadius         : MAX_INCINERATION_SPHERE_RADIUS;
    float  a_IncinerationDuration    : INCINERATION_DURATION;
    float  a_ElapsedTime             : ELAPSED_TIME;
};

struct VertexOutput
{
    float4                 v_Position            : SV_POSITION;
    float3                 v_WorldPos            : WORLD_POS;
    nointerpolation float3 v_SpherePosition      : INCINERATION_SPHERE_POSITION;
    float2                 v_TexCoords           : TEXCOORDS;
    nointerpolation float  v_IncinerationFactor  : INCINERATION_FACTOR;
    nointerpolation float  v_CurrentSphereRadius : CURRENT_INCINERATION_SPHERE_RADIUS;
    nointerpolation float  v_EpsilonSphereRadius : EPSILON_INCINERATION_SPHERE_RADIUS;
};

VertexOutput mainVS(VertexInput vsInput, TransformInput transformInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), transformInput.a_Transform);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);
    vsOutput.v_TexCoords = vsInput.a_TexCoords;
    vsOutput.v_SpherePosition = mul(float4(instInput.a_SpherePositionMeshSpace, 1.0), transformInput.a_Transform).xyz;
    vsOutput.v_IncinerationFactor = saturate(instInput.a_ElapsedTime / instInput.a_IncinerationDuration);
    vsOutput.v_CurrentSphereRadius = instInput.a_MaxSphereRadius * vsOutput.v_IncinerationFactor;
    
    const float incinerationSphereRadiusEpsilon = 0.15;
    vsOutput.v_EpsilonSphereRadius = vsOutput.v_CurrentSphereRadius * (1.0 - incinerationSphereRadiusEpsilon);
    
    return vsOutput;
}

Texture2D<float> t_IncinerationNoiseMap : register(t20);

void mainPS(VertexOutput psInput)
{
    const float distanceToIncinerationSphere = distance(psInput.v_WorldPos, psInput.v_SpherePosition);
    
    clip(distanceToIncinerationSphere - psInput.v_EpsilonSphereRadius);

    float incinerationAlpha = 1.0;
    if (distanceToIncinerationSphere < psInput.v_CurrentSphereRadius)
    {
        const float incinerationNoise = t_IncinerationNoiseMap.Sample(s_NearestWrap, psInput.v_TexCoords).r;
    
        const float incinerationEpsilon = 0.15; // To prevent incineration dash when incineration noise is 1.0
        const float fragmentIncineration = 1.0 - (psInput.v_IncinerationFactor + incinerationEpsilon);
        
        clip(fragmentIncineration - incinerationNoise);
    }
}