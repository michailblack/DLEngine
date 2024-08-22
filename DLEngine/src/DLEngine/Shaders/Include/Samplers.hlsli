#ifndef _SAMPLERS_HLSLI_
#define _SAMPLERS_HLSLI_

SamplerState s_ActiveSampler : register(s0);

SamplerState s_NearestWrap : register(s1);
SamplerState s_NearestClamp : register(s2);

SamplerState s_TrilinearWrap : register(s3);
SamplerState s_TrilinearClamp : register(s4);

SamplerState s_Anisotropic8Wrap : register(s5);
SamplerState s_Anisotropic8Clamp : register(s6);

SamplerComparisonState s_BilinearBorderCmp : register(s7);

#endif