cbuffer PerFrame : register(b0)
{
    float c_TimeMS;
    float c_TimeS;
    float2 c_Resolution;
    float2 c_MousePos;
};

cbuffer PerView : register(b1)
{
    float4x4 c_Projection;
    float4x4 c_InvProjection;
    float4x4 c_View;
    float4x4 c_InvView;
    float4x4 c_ViewProjection;
    float4x4 c_InvViewProjection;
    float4 c_CameraPosition;
};

struct PixelInput
{
    float4 v_ScreenSpacePos : SV_POSITION;
    float3 v_WorldPos       : WORLD_POS;
    float3 v_Normal         : NORMAL;
    float3 v_BaseColor      : BASE_COLOR;
    float3 v_AddColor       : ADD_COLOR;
};

struct PixelOutput
{
    float4 o_Color : SV_TARGET;
};

#include "Hologram.h.hlsl"

float hash4d(in float4 p) {
	p = frac(p * 0.1031);
    p += dot(p, p.zwyx + 31.32);
    return frac((p.x + p.y) * p.z - p.x * p.w);
}

float noise4d(in float4 p) {
    float4 cell = floor(p);
    float4 local = frac(p);
    local *= local * (3.0 - 2.0 * local);

    float ldbq = hash4d(cell);
    float rdbq = hash4d(cell + float4(1.0, 0.0, 0.0, 0.0));
    float ldfq = hash4d(cell + float4(0.0, 0.0, 1.0, 0.0));
    float rdfq = hash4d(cell + float4(1.0, 0.0, 1.0, 0.0));
    float lubq = hash4d(cell + float4(0.0, 1.0, 0.0, 0.0));
    float rubq = hash4d(cell + float4(1.0, 1.0, 0.0, 0.0));
    float lufq = hash4d(cell + float4(0.0, 1.0, 1.0, 0.0));
    float rufq = hash4d(cell + float4(1.0, 1.0, 1.0, 0.0));
    float ldbw = hash4d(cell + float4(0.0, 0.0, 0.0, 1.0));
    float rdbw = hash4d(cell + float4(1.0, 0.0, 0.0, 1.0));
    float ldfw = hash4d(cell + float4(0.0, 0.0, 1.0, 1.0));
    float rdfw = hash4d(cell + float4(1.0, 0.0, 1.0, 1.0));
    float lubw = hash4d(cell + float4(0.0, 1.0, 0.0, 1.0));
    float rubw = hash4d(cell + float4(1.0, 1.0, 0.0, 1.0));
    float lufw = hash4d(cell + float4(0.0, 1.0, 1.0, 1.0));
    float rufw = hash4d(cell + 1.0);

    return lerp(lerp(lerp(lerp(ldbq, rdbq, local.x),
                       lerp(lubq, rubq, local.x),
                       local.y),

                   lerp(lerp(ldfq, rdfq, local.x),
                       lerp(lufq, rufq, local.x),
                       local.y),

                   local.z),

               lerp(lerp(lerp(ldbw, rdbw, local.x),
                       lerp(lubw, rubw, local.x),
                       local.y),

                   lerp(lerp(ldfw, rdfw, local.x),
                       lerp(lufw, rufw, local.x),
                       local.y),

                   local.z),

               local.w);
}

float noise4d(in float4 p, uniform in uint octaves)
{
    float nscale = 1.0;
    float tscale = 0.0;
    float value = 0.0;

    [unroll(octaves)]
    for (uint octave=0; octave < octaves; octave++) {
        value += noise4d(p) * nscale;
        tscale += nscale;
        nscale *= 0.5;
        p *= 2.0;
    }

    return value / tscale;
}

float3 colorDistortion(float3 pos, float3 normal, float3 baseColor, float3 additionalColor)
{
    float baseWave = wave(pos, BASE_WAVE_INTERVAL, BASE_WAVE_SPEED, BASE_WAVE_THICKNESS, true);
    float addWave = wave(pos, ADD_WAVE_INTERVAL, ADD_WAVE_SPEED, ADD_WAVE_THICKNESS, false);

    float3 toCamera = normalize(c_CameraPosition.xyz - pos);
    float contourGlow = pow(1.0 - abs(dot(normal, toCamera)), 2);

    // when contourInterference is 0, ripple effect contourWave is added to contourGlow, otherwise contourWave is 1
    float contourWave = wave(pos, 0.1, 0.1, 0.05, false);
    float contourInterference = periodIntensity(c_TimeS, 4, 1);
    contourWave = lerp(contourWave, 1.0, contourInterference);
    // when contourWave is 0.0, contourGlow becomes darker, otherwise contourGlow color is plain, without ripple
    contourGlow = lerp(contourGlow / 10, contourGlow, contourWave);

    float3 color = baseColor * min(1, contourGlow + baseWave * 0.5);
    float colorNoise = sqrt(noise4d(float4(pos, frac(c_TimeS)) * 100, 1));
    color *= lerp(colorNoise, 1.0, contourInterference);
    
    color = lerp(color, additionalColor, addWave * 0.25);
    return color;
}

PixelOutput main(PixelInput psInput)
{
    PixelOutput psOutput;

    psOutput.o_Color = float4(colorDistortion(psInput.v_WorldPos, psInput.v_Normal, psInput.v_BaseColor, psInput.v_AddColor), 1.0);

    return psOutput;
}
