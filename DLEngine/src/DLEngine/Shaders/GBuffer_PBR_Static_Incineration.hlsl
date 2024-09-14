#include "Include/IncinerationParticle.hlsli"
#include "Include/Lighting.hlsli"

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
    float3x3               v_TangentToWorld      : TANGENT_TO_WORLD;
    float3                 v_WorldPos            : WORLD_POS;
    float3                 v_Normal              : NORMAL;
    nointerpolation float3 v_SpherePosition      : INCINERATION_SPHERE_POSITION;
    nointerpolation float3 v_Emission            : INCINERATION_EMISSION;
    float2                 v_TexCoords           : TEXCOORDS;
    nointerpolation uint2  v_InstanceUUID        : INSTANCE_UUID;
    nointerpolation float  v_IncinerationFactor  : INCINERATION_FACTOR;
    nointerpolation float  v_CurrentSphereRadius : CURRENT_INCINERATION_SPHERE_RADIUS;
    nointerpolation float  v_EpsilonSphereRadius : EPSILON_INCINERATION_SPHERE_RADIUS;
};

RWStructuredBuffer<IncinerationParticle> u_IncinerationParticles           : register(u5);
RWBuffer<uint>                           u_IncinerationParticleRangeBuffer : register(u6);

void SpawnIncinerationPartilce(float3 worldPos, float3 velocity, float3 emission, uint2 parentInstanceUUID)
{
    uint maxParticlesCount = 0, particleStride = 0;
    u_IncinerationParticles.GetDimensions(maxParticlesCount, particleStride);
    
    uint currentParticlesCount = 0;
    InterlockedAdd(u_IncinerationParticleRangeBuffer[ParticlesCountIndex], 1, currentParticlesCount);
    if (currentParticlesCount >= maxParticlesCount)
    {
        InterlockedAdd(u_IncinerationParticleRangeBuffer[ParticlesCountIndex], -1);
        return;
    }
    
    const uint particleOffset = u_IncinerationParticleRangeBuffer[ParticleOffsetIndex];
    const uint particleIndex = (particleOffset + currentParticlesCount) % maxParticlesCount;
    
    IncinerationParticle particle;
    particle.WorldPosition = worldPos;
    particle.Velocity = velocity;
    particle.Emission = emission;
    particle.ParentInstanceUUID = parentInstanceUUID;
    particle.LifetimeMS = frac(c_TimeS) * MaxIncinerationParticleLifetimeMS;
    particle.LifetimePassedMS = 0.0;
    
    u_IncinerationParticles[particleIndex] = particle;
}

static const uint ParticlesDiscardPercentage = 128;

VertexOutput mainVS(uint vertexID : SV_VertexID, VertexInput vsInput, TransformInput transformInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), transformInput.a_Transform);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);

    vsOutput.v_TexCoords = vsInput.a_TexCoords;
    vsOutput.v_InstanceUUID = instInput.a_InstanceUUID;

    const float3x3 normalMatrix = ConstructNormalMatrix(transformInput.a_Transform);
    vsOutput.v_Normal = mul(vsInput.a_Normal, normalMatrix);
    
    const float3 T = mul(vsInput.a_Tangent, normalMatrix);
    const float3 B = mul(vsInput.a_Bitangent, normalMatrix);
    vsOutput.v_TangentToWorld = float3x3(T, B, vsOutput.v_Normal);

    vsOutput.v_SpherePosition = mul(float4(instInput.a_SpherePositionMeshSpace, 1.0), transformInput.a_Transform).xyz;
    vsOutput.v_Emission = instInput.a_ParticleEmission;
    vsOutput.v_IncinerationFactor = saturate(instInput.a_ElapsedTime / instInput.a_IncinerationDuration);
    vsOutput.v_CurrentSphereRadius = instInput.a_MaxSphereRadius * vsOutput.v_IncinerationFactor;
    
    const float incinerationSphereRadiusEpsilon = 0.15;
    vsOutput.v_EpsilonSphereRadius = vsOutput.v_CurrentSphereRadius * (1.0 - incinerationSphereRadiusEpsilon);
    
    const float previousFrameIncinerationFactor = saturate((instInput.a_ElapsedTime - c_DeltaTimeMS) / instInput.a_IncinerationDuration);
    const float previousFrameSphereRadius = instInput.a_MaxSphereRadius * previousFrameIncinerationFactor;
    const float distanceToIncinerationSphere = distance(vsOutput.v_WorldPos, vsOutput.v_SpherePosition);
    if (distanceToIncinerationSphere > previousFrameSphereRadius && distanceToIncinerationSphere < vsOutput.v_CurrentSphereRadius)
    {
        if (vertexID % ParticlesDiscardPercentage == 0)
        {
            const float3 velocity = normalize(vsOutput.v_Normal) * 1.5;
            SpawnIncinerationPartilce(vsOutput.v_WorldPos, velocity, vsOutput.v_Emission, vsOutput.v_InstanceUUID);
        }
    }
    
    return vsOutput;
}

struct PixelOutput
{
    float4 o_Albedo                 : SV_TARGET0;
    float2 o_MetalnessRoughness     : SV_TARGET1;
    float4 o_GeometrySurfaceNormals : SV_TARGET2;
    float4 o_Emission               : SV_TARGET3;
    uint2  o_InstanceUUID           : SV_TARGET4;
};

Texture2D<float> t_IncinerationNoiseMap : register(t20);

PixelOutput mainPS(VertexOutput psInput)
{
    const float distanceToIncinerationSphere = distance(psInput.v_WorldPos, psInput.v_SpherePosition);
    const Surface surface = CalculatePBR_Surface(psInput.v_TexCoords, psInput.v_Normal, psInput.v_TangentToWorld);
    
    clip(distanceToIncinerationSphere - psInput.v_EpsilonSphereRadius);

    float incinerationAlpha = 1.0;
    if (distanceToIncinerationSphere < psInput.v_CurrentSphereRadius)
    {
        const float incinerationNoise = t_IncinerationNoiseMap.Sample(s_NearestWrap, psInput.v_TexCoords).r;
    
        const float incinerationEpsilon = 0.15; // To prevent incineration dash when incineration noise is 1.0
        const float fragmentIncineration = 1.0 - (psInput.v_IncinerationFactor + incinerationEpsilon);
        
        clip(fragmentIncineration - incinerationNoise);
        
        const float falloff = incinerationEpsilon / Epsilon;
        incinerationAlpha = saturate((fragmentIncineration - incinerationNoise) / max(fwidth(fragmentIncineration), Epsilon) / falloff);
    }
        
    incinerationAlpha *= smoothstep(psInput.v_EpsilonSphereRadius, psInput.v_CurrentSphereRadius, distanceToIncinerationSphere);
    
    PixelOutput psOutput;
    psOutput.o_Albedo = float4(surface.Albedo, 1.0);
    psOutput.o_MetalnessRoughness = float2(surface.Metalness, surface.Roughness);
    psOutput.o_GeometrySurfaceNormals = float4(packOctahedron(normalize(surface.GeometryNormal)), packOctahedron(surface.SurfaceNormal));
    psOutput.o_InstanceUUID = psInput.v_InstanceUUID;
    psOutput.o_Emission = lerp(float4(psInput.v_Emission, 1.0), float4(0.0, 0.0, 0.0, 1.0), incinerationAlpha);
    
    return psOutput;
}