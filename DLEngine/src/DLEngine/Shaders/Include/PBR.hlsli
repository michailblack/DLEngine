#ifndef _PBR_HLSLI_
#define _PBR_HLSLI_

#include "Common.hlsli"

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float NDF_GGX(float NdotH, float roughness)
{
    const float alpha = roughness * roughness;
    const float alphaSq = alpha * alpha;
    
    const float denom = NdotH * NdotH * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Shlick-GGX term
float ShlickG1(float NdotD, float k)
{
    return NdotD / (NdotD * (1.0 - k) + k);
}

// Shlick-GGX approximation of geometric attenuation function
float ShlickGGX(float NdotV, float NdotL, float roughness)
{
    // Epic's roughness remapping for analytic lights
    const float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    
    return ShlickG1(NdotV, k) * ShlickG1(NdotL, k);
}

// Shlick's approximation of the Fresnel factor.
float3 FresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

#endif