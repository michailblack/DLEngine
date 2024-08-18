static const float GoldenRatio = (1.0 + sqrt(5)) / 2.0;

float3 FibonacciHemispherePoint(uint index, uint pointsCount)
{
    const float theta = 2.0 * PI * float(index) / GoldenRatio;
    const float phiCos = 1.0 - (float(index) + 0.5) / float(pointsCount);
    const float phiSin = sqrt(1.0 - phiCos * phiCos);
    const float thetaCos = cos(theta);
    const float thetaSin = sin(theta);
    
    return float3(phiSin * thetaCos, phiSin * thetaSin, phiCos);
}

float RandomVanDeCorput(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 RandomHammersley(uint index, uint count)
{
    return float2(float(index) / float(count), RandomVanDeCorput(index));
}

float3 ImportanceSampleGGX(float2 random, float roughness)
{
    const float a = roughness * roughness;
    
    const float phi = 2.0 * PI * random.x;
    const float cosTheta = sqrt((1.0 - random.y) / (1.0 + (a * a - 1.0) * random.y));
    const float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

float3 ImportanceSampleGGX(out float NdotH, uint index, uint count, float roughness, float3x3 rotation)
{
    float3 H = ImportanceSampleGGX(RandomHammersley(index, count), roughness);
    NdotH = H.z;
    return mul(H, rotation);
}

// Determing which mip level to read in cubemap sampling with uniform/importance sampling
float HemisphereMip(float sampleProbability, float cubemapSize)
{
    float hemisphereTexels = cubemapSize * cubemapSize * 3;
    float log4 = 0.5 * log2(sampleProbability * hemisphereTexels);
    return log4;
}