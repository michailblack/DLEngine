#ifndef _SAMPLERS_HLSLI_
#define _SAMPLERS_HLSLI_

SamplerState s_ActiveSampler : register(s0);

SamplerState s_NearestWrap  : register(s1);
SamplerState s_NearestClamp : register(s2);

SamplerState s_BilinearWrap  : register(s3);
SamplerState s_BilinearClamp : register(s4);

SamplerState s_TrilinearWrap  : register(s5);
SamplerState s_TrilinearClamp : register(s6);

SamplerState s_Anisotropic8Wrap  : register(s7);
SamplerState s_Anisotropic8Clamp : register(s8);

SamplerComparisonState s_BilinearBorderCmp : register(s9);

#endif