#ifndef _INCINERATION_PARTICLE_HLSLI_
#define _INCINERATION_PARTICLE_HLSLI_

struct IncinerationParticle
{
    float3 WorldPosition;
    float3 Velocity;
    float3 Emission;
    uint2 ParentInstanceUUID;
    float LifetimeMS;
    float LifetimePassedMS;
};

static const float MaxIncinerationParticleLifetimeMS = 4000.0;

static const uint ParticlesCountIndex = 0;
static const uint ParticleOffsetIndex = 1;
static const uint ExpiredParticlesCountIndex = 2;
static const uint ThreadGroupCountXIndex = 3;

static const float ParticleBillboardWorldSize = 0.04;
static const float PartilceInfluenceBillboardWorldSize = ParticleBillboardWorldSize * 10.0;
static const float ParticlePointLightWorldRadius = sqrt(3.0) * ParticleBillboardWorldSize * 0.5;

static const float2 BillboardVertexDirections[4] =
{
    normalize(float2(-1.0, -1.0)),
    normalize(float2(-1.0,  1.0)),
    normalize(float2( 1.0,  1.0)),
    normalize(float2( 1.0, -1.0))
};

static const float2 BillboardTexCoords[4] =
{
    float2(0.0, 1.0),
    float2(0.0, 0.0),
    float2(1.0, 0.0),
    float2(1.0, 1.0)
};

#endif