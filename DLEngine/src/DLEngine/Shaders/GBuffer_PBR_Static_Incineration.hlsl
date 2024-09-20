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
    uint   a_ParicleDiscardDivisor   : PARTICLE_DISCARD_DIVISOR;
};

struct PixelInput
{
    float4                 v_Position             : SV_POSITION;
    float3x3               v_TangentToWorld       : TANGENT_TO_WORLD;
    float3                 v_WorldPos             : WORLD_POS;
    float3                 v_Normal               : NORMAL;
    nointerpolation float3 v_SpherePosition       : INCINERATION_SPHERE_POSITION;
    nointerpolation float3 v_Emission             : INCINERATION_EMISSION;
    float2                 v_TexCoords            : TEXCOORDS;
    nointerpolation uint2  v_InstanceUUID         : INSTANCE_UUID;
    nointerpolation float  v_IncinerationFactor   : INCINERATION_FACTOR;
    nointerpolation float  v_CurrentSphereRadius  : CURRENT_INCINERATION_SPHERE_RADIUS;
    nointerpolation float  v_EpsilonSphereRadius  : EPSILON_INCINERATION_SPHERE_RADIUS;
};

struct VertexOutput
{
    PixelInput            v_PixelInput             : PIXEL_INPUT;
    nointerpolation float v_PreviousSphereRadius   : PREVIOUS_INCINERATION_SPHERE_RADIUS;
    nointerpolation uint  v_ParticleDiscardDivisor : PARTICLE_DISCARD_DIVISOR;
};

VertexOutput mainVS(VertexInput vsInput, TransformInput transformInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    const float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), transformInput.a_Transform);

    vsOutput.v_PixelInput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_PixelInput.v_Position = mul(vertexPos, c_ViewProjection);

    const float3x3 normalMatrix = ConstructNormalMatrix(transformInput.a_Transform);
    vsOutput.v_PixelInput.v_Normal = mul(vsInput.a_Normal, normalMatrix);
    
    const float3 T = mul(vsInput.a_Tangent, normalMatrix);
    const float3 B = mul(vsInput.a_Bitangent, normalMatrix);
    vsOutput.v_PixelInput.v_TangentToWorld = float3x3(T, B, vsOutput.v_PixelInput.v_Normal);

    vsOutput.v_PixelInput.v_SpherePosition = mul(float4(instInput.a_SpherePositionMeshSpace, 1.0), transformInput.a_Transform).xyz;
    vsOutput.v_PixelInput.v_Emission = instInput.a_ParticleEmission;
    vsOutput.v_PixelInput.v_IncinerationFactor = saturate(instInput.a_ElapsedTime / instInput.a_IncinerationDuration);
    vsOutput.v_PixelInput.v_CurrentSphereRadius = instInput.a_MaxSphereRadius * vsOutput.v_PixelInput.v_IncinerationFactor;
    
    const float incinerationSphereRadiusEpsilon = 0.15;
    vsOutput.v_PixelInput.v_EpsilonSphereRadius = vsOutput.v_PixelInput.v_CurrentSphereRadius * (1.0 - incinerationSphereRadiusEpsilon);
    
    const float previousFrameIncinerationFactor = saturate((instInput.a_ElapsedTime - c_DeltaTimeMS) / instInput.a_IncinerationDuration);
    vsOutput.v_PreviousSphereRadius = instInput.a_MaxSphereRadius * previousFrameIncinerationFactor;
    
    vsOutput.v_PixelInput.v_TexCoords = vsInput.a_TexCoords;
    vsOutput.v_PixelInput.v_InstanceUUID = instInput.a_InstanceUUID;
    vsOutput.v_ParticleDiscardDivisor = instInput.a_ParicleDiscardDivisor;
    
    return vsOutput;
}

struct HullPatchOutput
{
    float v_EdgeTessFactor[3] : SV_TessFactor;
    float v_InsideTessFactor  : SV_InsideTessFactor;
};

static const uint NUM_CONTROL_POINTS = 3;
static const float MAX_TESS_FACTOR = 8.0;
static const float MIN_TESS_FACTOR = 1.0;

HullPatchOutput CalcHSPatchConstants(InputPatch<VertexOutput, NUM_CONTROL_POINTS> inputPatch)
{
    HullPatchOutput patchOutput;

    const float AB = length(inputPatch[1].v_PixelInput.v_WorldPos - inputPatch[0].v_PixelInput.v_WorldPos);
    const float BC = length(inputPatch[2].v_PixelInput.v_WorldPos - inputPatch[1].v_PixelInput.v_WorldPos);
    const float CA = length(inputPatch[0].v_PixelInput.v_WorldPos - inputPatch[2].v_PixelInput.v_WorldPos);
    const float maxEdgeLength = max(AB, max(BC, CA));

    patchOutput.v_EdgeTessFactor[0] = lerp(MIN_TESS_FACTOR, MAX_TESS_FACTOR, AB / maxEdgeLength);
    patchOutput.v_EdgeTessFactor[1] = lerp(MIN_TESS_FACTOR, MAX_TESS_FACTOR, BC / maxEdgeLength);
    patchOutput.v_EdgeTessFactor[2] = lerp(MIN_TESS_FACTOR, MAX_TESS_FACTOR, CA / maxEdgeLength);
    patchOutput.v_InsideTessFactor = MAX_TESS_FACTOR;

    return patchOutput;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(MAX_TESS_FACTOR)]
VertexOutput mainHS(InputPatch<VertexOutput, NUM_CONTROL_POINTS> inputPatch, uint pointID : SV_OutputControlPointID)
{
    return inputPatch[pointID];
}

struct DomainOutput
{
    VertexOutput         v_VertexOutput        : VERTEXT_OUTPUT;
    nointerpolation uint v_TessellatedVertexID : TESSELLATED_VERTEX_ID;
};

float4 InterpolateBarycentric(float3 bary, float4 a, float4 b, float4 c)
{
    return a * bary.x + b * bary.y + c * bary.z;
}

float3 InterpolateBarycentric(float3 bary, float3 a, float3 b, float3 c)
{
    return a * bary.x + b * bary.y + c * bary.z;
}

float2 InterpolateBarycentric(float3 bary, float2 a, float2 b, float2 c)
{
    return a * bary.x + b * bary.y + c * bary.z;
}

[domain("tri")]
DomainOutput mainDS(const OutputPatch<VertexOutput, NUM_CONTROL_POINTS> patch, HullPatchOutput patchConstantData, float3 bary : SV_DomainLocation, uint primitiveID : SV_PrimitiveID)
{
    DomainOutput dsOutput;
    
    dsOutput.v_VertexOutput.v_PixelInput.v_Position = InterpolateBarycentric(bary, patch[0].v_PixelInput.v_Position, patch[1].v_PixelInput.v_Position, patch[2].v_PixelInput.v_Position);
    dsOutput.v_VertexOutput.v_PixelInput.v_WorldPos = InterpolateBarycentric(bary, patch[0].v_PixelInput.v_WorldPos, patch[1].v_PixelInput.v_WorldPos, patch[2].v_PixelInput.v_WorldPos);
    dsOutput.v_VertexOutput.v_PixelInput.v_Normal = InterpolateBarycentric(bary, patch[0].v_PixelInput.v_Normal, patch[1].v_PixelInput.v_Normal, patch[2].v_PixelInput.v_Normal);
    dsOutput.v_VertexOutput.v_PixelInput.v_TexCoords = InterpolateBarycentric(bary, patch[0].v_PixelInput.v_TexCoords, patch[1].v_PixelInput.v_TexCoords, patch[2].v_PixelInput.v_TexCoords);
    
    [unroll(3)]
    for (uint i = 0; i < 3; ++i)
    {
        dsOutput.v_VertexOutput.v_PixelInput.v_TangentToWorld[i].xyz = InterpolateBarycentric(bary,
            patch[0].v_PixelInput.v_TangentToWorld[i].xyz,
            patch[1].v_PixelInput.v_TangentToWorld[i].xyz,
            patch[2].v_PixelInput.v_TangentToWorld[i].xyz
        );
    }
    
    dsOutput.v_VertexOutput.v_PixelInput.v_Emission = patch[0].v_PixelInput.v_Emission;
    dsOutput.v_VertexOutput.v_PixelInput.v_SpherePosition = patch[0].v_PixelInput.v_SpherePosition;
    dsOutput.v_VertexOutput.v_PixelInput.v_InstanceUUID = patch[0].v_PixelInput.v_InstanceUUID;
    dsOutput.v_VertexOutput.v_PixelInput.v_IncinerationFactor = patch[0].v_PixelInput.v_IncinerationFactor;
    dsOutput.v_VertexOutput.v_PixelInput.v_CurrentSphereRadius = patch[0].v_PixelInput.v_CurrentSphereRadius;
    dsOutput.v_VertexOutput.v_PixelInput.v_EpsilonSphereRadius = patch[0].v_PixelInput.v_EpsilonSphereRadius;
    
    dsOutput.v_VertexOutput.v_PreviousSphereRadius = patch[0].v_PreviousSphereRadius;
    dsOutput.v_VertexOutput.v_ParticleDiscardDivisor = patch[0].v_ParticleDiscardDivisor;
    
    const uint n = uint(patchConstantData.v_InsideTessFactor);
    const uint j = uint(bary.y * n + 0.5);
    const uint k = uint(bary.z * n + 0.5);

    dsOutput.v_TessellatedVertexID = primitiveID + (n - j) * (n - j + 1) / 2 + k;

    return dsOutput;
}

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
    particle.LifetimeMS = frac(c_TimeMS) * MaxIncinerationParticleLifetimeMS;
    particle.LifetimePassedMS = 0.0;
    
    u_IncinerationParticles[particleIndex] = particle;
}

[maxvertexcount(NUM_CONTROL_POINTS)]
void mainGS(triangle DomainOutput input[3], inout TriangleStream<PixelInput> output)
{
    const float3 A = input[0].v_VertexOutput.v_PixelInput.v_WorldPos;
    const float3 B = input[1].v_VertexOutput.v_PixelInput.v_WorldPos;
    const float3 C = input[2].v_VertexOutput.v_PixelInput.v_WorldPos;
    const float3 triangleCenter = (A + B + C) / 3.0;
    
    const float3 incinerationSpherePosition = input[0].v_VertexOutput.v_PixelInput.v_SpherePosition;
    const float currentSphereRadius = input[0].v_VertexOutput.v_PixelInput.v_CurrentSphereRadius;
    const float previousSphereRadius = input[0].v_VertexOutput.v_PreviousSphereRadius;
    
    const float distanceToIncinerationSphere = distance(triangleCenter, incinerationSpherePosition);
    if (distanceToIncinerationSphere > previousSphereRadius && distanceToIncinerationSphere < currentSphereRadius)
    {
        const uint particleDiscardDivisor = input[0].v_VertexOutput.v_ParticleDiscardDivisor;
        
        bool shouldSpawnParticle = input[0].v_TessellatedVertexID % particleDiscardDivisor == 0 &&
            input[1].v_TessellatedVertexID % particleDiscardDivisor == 0 &&
            input[2].v_TessellatedVertexID % particleDiscardDivisor == 0;
        
        if (shouldSpawnParticle)
        {
            const float3 emission = input[0].v_VertexOutput.v_PixelInput.v_Emission;
            const uint2 parentInstanceUUID = input[0].v_VertexOutput.v_PixelInput.v_InstanceUUID;
            
            const float3 normalA = input[0].v_VertexOutput.v_PixelInput.v_Normal;
            const float3 normalB = input[1].v_VertexOutput.v_PixelInput.v_Normal;
            const float3 normalC = input[2].v_VertexOutput.v_PixelInput.v_Normal;
            
            const float3 triangleNormal = normalize(normalA + normalB + normalC);
            const float3 velocity = triangleNormal * 2.0;
            
            SpawnIncinerationPartilce(triangleCenter, velocity, emission, parentInstanceUUID);
        }
    }
    
    [unroll(3)]
    for (uint i = 0; i < 3; ++i)
        output.Append(input[i].v_VertexOutput.v_PixelInput);
    
    output.RestartStrip();
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

PixelOutput mainPS(PixelInput psInput)
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