struct PixelInput
{
    float4 v_ScreenSpacePos : SV_POSITION;
};

struct PixelOutput
{
    float4 o_Color : SV_TARGET;
};

PixelOutput main(PixelInput psInput)
{
    PixelOutput psOutput;

    psOutput.o_Color = float4(1.0, 1.0, 0.0, 1.0);

    return psOutput;
}
