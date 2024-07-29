static const float PI = 3.14159265359;

float TrowbridgeReitzNDF(float NdotH, float rough4)
{
    float denom = NdotH * NdotH * (rough4 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return rough4 / denom;
}

float SmithGAF(float NdotV, float NdotL, float rough4)
{
    return 2.0 / (sqrt(1.0 + (rough4 * (1.0 - NdotV) / NdotV)) * sqrt(1.0 + (rough4 * (1.0 - NdotL) / NdotL)));
}

float3 FresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}
