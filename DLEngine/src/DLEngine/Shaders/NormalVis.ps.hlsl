struct PixelInput
{
    float4 v_ScreenSpacePos : SV_POSITION;
    float3 v_Normal         : NORMAL;
};

struct PixelOutput
{
    float4 o_Color : SV_TARGET;
};

PixelOutput main(PixelInput psInput)
{
    PixelOutput psOutput;

    float3 visualizedNormal = psInput.v_Normal * 0.5 + 0.5;
    psOutput.o_Color = float4(visualizedNormal, 1.0);

    return psOutput;
}
