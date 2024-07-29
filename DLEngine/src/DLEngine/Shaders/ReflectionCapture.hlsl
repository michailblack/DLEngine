#include "Input.hlsli"
#include "BRDF.hlsli"

cbuffer ReflectionCapture : register(b3)
{
    float4x4 c_View;
    float4x4 c_Projection;
    uint c_EnvironmentMapSize;
    float c_Roughness;
};

struct VertexOutput
{
    float4 v_Position      : SV_POSITION;
    float3 v_CubemapSample : CUBEMAP_SAMPLE;
    float2 v_TexCoords     : TEXCOORDS;
};

VertexOutput mainVS(VertexInput vsInput)
{
    VertexOutput vsOutput;
    vsOutput.v_Position = mul(float4(vsInput.a_Position, 1.0), c_View);
    vsOutput.v_Position = mul(vsOutput.v_Position, c_Projection);

    vsOutput.v_CubemapSample = vsInput.a_Position.xyz;
    vsOutput.v_TexCoords = vsInput.a_TexCoords;

    return vsOutput;
}

SamplerState s_AnisotropicClamp : register(s0);

StructuredBuffer<float3> t_HemispherePoints : register(t5);
TextureCube t_EnvironmentMap                : register(t6);

void FrisvadBasis(out float3 right, out float3 up, in float3 dir)
{
    const float k = 1.0 / max(1.0 + dir.z, 1e-5);
    const float a = dir.y * k;
    const float b = dir.y * a;
    const float c = -dir.x * a;
    right = float3(dir.z + b, c, -dir.x);
    up = float3(c, 1.0 - b, -dir.y);
}

float3x3 FrisvadBasis(float3 dir)
{
    float3x3 rotation;
    rotation[2] = dir;
    FrisvadBasis(rotation[0], rotation[1], dir);
    return rotation;
}

float randomVanDeCorput(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// random2D from random1D
float2 randomHammersley(float i, float N)
{
    return float2(i / N, randomVanDeCorput(i));
}

// GGX importance sampling, returns microsurface normal (half-vector)
// rough4 is initial roughness value in power of 4 
float3 randomGGX(float2 random, float rough4)
{
    float phi = 2.0 * PI * random.x;
    float cosTheta = sqrt((1.0 - random.y) / (1.0 + (rough4 - 1.0) * random.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 dir;
    dir.x = cos(phi) * sinTheta;
    dir.y = sin(phi) * sinTheta;
    dir.z = cosTheta;
    return dir;
}

// GGX importance sampling, returns microsurface normal (half-vector)
// rough4 is initial roughness value in power of 4
float3 randomGGX(out float NdotH, uint index, uint N, float rough4, float3x3 rotation)
{
    float3 H = randomGGX(randomHammersley(index, N), rough4);
    NdotH = H.z;
    return mul(H, rotation);
}

// Determing which mip level to read in cubemap sampling with uniform/importance sampling
float hemisphereMip(float sampleProbability, float cubemapSize)
{
    float hemisphereTexels = cubemapSize * cubemapSize * 3;
    float log4 = 0.5 * log2(sampleProbability * hemisphereTexels);
    return log4;
}

float4 mainPSDiffuseIrradiance(VertexOutput psInput) : SV_TARGET0
{
    uint numSamples;
    uint stride;
    t_HemispherePoints.GetDimensions(numSamples, stride);

    const float3 normal = normalize(psInput.v_CubemapSample);
    const float3x3 rotation = FrisvadBasis(normal);
    const float mipLevel = hemisphereMip(1.0 / numSamples, c_EnvironmentMapSize);

    float3 irradiance = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < numSamples; ++i)
    {
        const float3 location = mul(t_HemispherePoints[i], rotation);
        const float3 envIrradiance = t_EnvironmentMap.SampleLevel(s_AnisotropicClamp, location, mipLevel).rgb;
        const float NdotL = dot(normal, normalize(location));

        irradiance += envIrradiance * NdotL * (1.0 - FresnelSchlick(float3(0.04, 0.04, 0.04), NdotL)) / PI;
    }

    irradiance *= 2.0 * PI / float(numSamples);

    return float4(irradiance, 1.0);
}

float4 mainPSSpecularIrradiance(VertexOutput psInput) : SV_TARGET0
{
    uint numSamples;
    uint stride;
    t_HemispherePoints.GetDimensions(numSamples, stride);

    const float threshold = 1e-5;
    const float rough4 = pow(c_Roughness, 4);

    const float3 normal = normalize(psInput.v_CubemapSample);
    const float3x3 rotation = FrisvadBasis(normal);

    uint usedSamples = 0;
    float3 irradiance = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < numSamples; ++i)
    {
        float NdotH;
        const float3 halfVec = randomGGX(NdotH, i, numSamples, rough4, rotation);
        const float3 location = reflect(-normal, halfVec);

        if (dot(normal, normalize(location)) < threshold)
            continue;

        const float sizeFactor = 2.0 / (PI * numSamples * TrowbridgeReitzNDF(NdotH, rough4));
        const float mipLevel = hemisphereMip(sizeFactor, c_EnvironmentMapSize);

        irradiance += t_EnvironmentMap.SampleLevel(s_AnisotropicClamp, location, mipLevel).rgb;
        ++usedSamples;
    }

    irradiance /= float(usedSamples);

    return float4(irradiance, 1.0);
}

float GeometrySchlickGGX(float NdotV, float rough2)
{
    const float k = rough2 / 2.0;

    const float nom   = NdotV;
    const float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float UnrealSmithG(float NdotV, float NdotL, float rough2)
{
    float ggx2 = GeometrySchlickGGX(NdotV, rough2);
    float ggx1 = GeometrySchlickGGX(NdotL, rough2);

    return ggx1 * ggx2;
}

float2 mainPSSpecularFactor(VertexOutput psInput) : SV_TARGET0
{
    uint numSamples;
    uint stride;
    t_HemispherePoints.GetDimensions(numSamples, stride);

    // Using +Z cube face to draw so need to mirror u coordinate
    // to move (0, 0) to the top left from the top right
    const float rough = 1.0 - psInput.v_TexCoords.x;
    const float NdotV = psInput.v_TexCoords.y;

    const float rough4 = pow(rough, 4);
    const float3 view = float3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

    uint usedSamples = 0;
    float specR = 0.0;
    float specG = 0.0;
    for (uint i = 0; i < numSamples; ++i)
    {
        const float threshold = 1e-5;

        const float3 halfVec = normalize(randomGGX(randomHammersley(i, numSamples), rough4));
        const float NdotH = halfVec.z;
        if (NdotH < threshold)
            continue;

        const float HdotV = dot(halfVec, view);
        if (HdotV < threshold)
            continue;

        const float3 location = normalize(reflect(-view, halfVec));
        const float NdotL = location.z;
        if (NdotL < threshold)
            continue;

        if (NdotL > 0.0)
        {
            //const float G = UnrealSmithG(NdotV, NdotL, rough * rough);
            const float G = SmithGAF(NdotV, NdotL, rough4);
            const float visabilityG = (G * HdotV) / (NdotV * NdotH);
            const float reflectanceFresnel = pow(1.0 - HdotV, 5);

            specR += visabilityG * (1.0 - reflectanceFresnel);
            specG += visabilityG * reflectanceFresnel;
            ++usedSamples;
        }
    }

    return float2(specR, specG) / float(usedSamples);
}
