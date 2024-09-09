#ifndef _GBUFFER_RESOURCES_HLSLI_
#define _GBUFFER_RESOURCES_HLSLI_

Texture2D<float3> t_GBufferAlbedo                 : register(t16);
Texture2D<float2> t_GBufferMetalnessRoughness     : register(t17);
Texture2D<float4> t_GBufferGeometrySurfaceNormals : register(t18);
Texture2D<float3> t_GBufferEmission               : register(t19);

#endif