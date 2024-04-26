struct VertexInput
{
    float3 a_Pos   : POSITION;
    float3 a_Color : COLOR;
};

struct VertexOutput
{
    float3 v_Color : COLOR;
    float4 v_Pos   : SV_POSITION;
};

VertexOutput main(VertexInput vsInput)
{
    VertexOutput vsOutput;
    vsOutput.v_Pos = float4(vsInput.a_Pos, 1.0);
    vsOutput.v_Color = vsInput.a_Color;

    return vsOutput;
}