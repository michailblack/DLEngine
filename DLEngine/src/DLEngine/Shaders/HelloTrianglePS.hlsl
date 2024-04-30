cbuffer PerFrame
{
    float2 a_Resolution;
    float a_Time;
};

struct PixelInput
{
    float3 v_Color : COLOR;
    float4 v_ScreenSpacePos : SV_Position;
};

struct PixelOutput
{
    float4 v_Color : SV_TARGET;
};

float noise(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

PixelOutput main(PixelInput psInput)
{
    PixelOutput psOutput;

#ifdef HelloShaderToy
    float4 pixelCoord = psInput.v_ScreenSpacePos;

    float2 uv = pixelCoord.xy / a_Resolution.xy;
    
    const float brightness = 1.2;
    const float blobiness = 0.9;
    const float particles = 140.0;
    const float limit = 70.0;
    const float energy = 1.0 * 0.75;

    const float2 position = pixelCoord.xy / a_Resolution.x;
    const float t = a_Time * energy;
    
    float a = 0.0;
    float b = 0.0;
    float c = 0.0;

    const float2 center = float2(0.5, 0.5 * (a_Resolution.y / a_Resolution.x));

    const float lim = particles / limit;
    const float step = 1.0 / particles;
    float n = 0.0;

    [unroll(40)]
    for (float i = 0.0; i <= 1.0; i += 0.025)
    {
        if (i <= lim)
        {
            const float2 np = float2(n, 0.0);
            
            const float na = noise(np * 1.1);
            const float nb = noise(np * 2.8);
            const float nc = noise(np * 0.7);
            const float nd = noise(np * 3.2);

            float2 pos = center;
            pos.x += sin(t * na) * cos(t * nb) * tan(t * na * 0.15) * 0.3;
            pos.y += tan(t * nc) * sin(t * nd) * 0.1;
            
            const float d = pow(1.6 * na / length(pos - position), blobiness);
            
            if (i < lim * 0.3333)
                a += d;
            else if (i < lim * 0.5)
                b += d;
            else
                c += d;

            n += step;
        }
    }

    float3 col = float3(a * 25.5, 0.0, a * b) * 0.0001 * brightness;
    psOutput.v_Color = float4(col, 1.0);
#else
    psOutput.v_Color = float4(psInput.v_Color, 1.0);
#endif

    return psOutput;
}