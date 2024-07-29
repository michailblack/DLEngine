static const float PI = 3.14159265359;
static const float THRESHOLD = 1e-5;

float TrowbridgeReitzNDF(float NdotH, float rough4)
{
    const float rough = max(rough4, THRESHOLD);

    float denom = NdotH * NdotH * (rough - 1.0) + 1.0;
    denom = PI * denom * denom;

    return rough / denom;
}

float SmithGAF(float NdotV, float NdotL, float rough4)
{
    return 2.0 / (sqrt(1.0 + (rough4 * (1.0 - NdotV) / NdotV)) * sqrt(1.0 + (rough4 * (1.0 - NdotL) / NdotL)));
}

float3 FresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}
