cbuffer PerFrame : register(b0)
{
    float c_TimeMS;
    float c_TimeS;
    float2 c_Resolution;
    float2 c_MousePos;
};

cbuffer PerView : register(b1)
{
    row_major float4x4 c_Projection;
    row_major float4x4 c_InvProjection;
    row_major float4x4 c_View;
    row_major float4x4 c_InvView;
    row_major float4x4 c_ViewProjection;
    row_major float4x4 c_InvViewProjection;
    float4 c_CameraPosition;
};

cbuffer PerDraw : register(b2)
{
    row_major float4x4 c_MeshToModel;
    row_major float4x4 c_ModelToMesh;
};

struct VertexInput
{
    float3 a_Position  : POSITION;
    float3 a_Normal    : NORMAL;
    float3 a_Tangent   : TANGENT;
    float3 a_Bitangent : BITANGENT;  
    float2 a_TexCoords : TEXCOORDS;
};

struct InstanceInput
{
    row_major float4x4 a_ModelToWorld : TRANSFORM;
    float3 a_BaseColor                : BASE_COLOR;
    float3 a_AddColor                 : ADD_COLOR;
};

struct VertexOutput
{
    float4 o_Position  : SV_POSITION;
    float3 o_WorldPos  : WORLD_POS;
    float3 o_Normal    : NORMAL;
    float3 o_BaseColor : BASE_COLOR;
    float3 o_AddColor  : ADD_COLOR;
};

float distanceIntensity(float value, float target, float fade)
{
    return min(1.0, abs(value - target) / fade);
}

float periodIntensity(float value, float period, float fade)
{
    float target = round(value / period) * period;
    return distanceIntensity(value, target, fade);
}

float wave(float3 pos, float waveInterval, float waveYSpeed, float waveThickness, uniform bool distort)
{
    if (distort)
    {
        const float WAVE_XZ_SPEED = 3.0;
        const float WAVE_DISTORTION_SIZE = 0.035;
        const float WAVE_OSCILLATING_TIME = 4;

        float distortionSign = abs(frac(c_TimeS / WAVE_OSCILLATING_TIME) - 0.5) * 4 - 1;
        float2 distortion = sin(pos.xz / WAVE_DISTORTION_SIZE + c_TimeS * WAVE_XZ_SPEED) * WAVE_DISTORTION_SIZE * distortionSign;
        pos.y -= (distortion.x + distortion.y);
    }

    pos.y -= c_TimeS * waveYSpeed;

    float intensity = 1.0 - periodIntensity(pos.y, waveInterval, waveThickness);
    return intensity;
}

static const float BLUE_WAVE_INTERVAL = 0.8;
static const float BLUE_WAVE_SPEED = 0.25;
static const float BLUE_WAVE_THICKNESS = 0.05;

static const float RED_WAVE_INTERVAL = 10;
static const float RED_WAVE_SPEED = 2;
static const float RED_WAVE_THICKNESS = 0.2;

float3 vertexDistortion(float3 pos, float3 normal)
{
    float3 offset = 0.0;
    offset += normal * 0.025 * wave(pos, BLUE_WAVE_INTERVAL, BLUE_WAVE_SPEED, BLUE_WAVE_THICKNESS, true);
    offset += normal * 0.05 * wave(pos, RED_WAVE_INTERVAL, RED_WAVE_SPEED, RED_WAVE_THICKNESS, false);
    return offset;
}

VertexOutput main(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4x4 meshToWorld = mul(c_MeshToModel, instInput.a_ModelToWorld);
    
    float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), meshToWorld);
    
    float3 offset = vertexDistortion(vertexPos.xyz, vsInput.a_Normal);

    vertexPos += float4(offset, 0.0);

    vsOutput.o_Position = mul(vertexPos, c_ViewProjection);
    
    vsOutput.o_WorldPos = vertexPos;

    float3 axisX = normalize(meshToWorld[0].xyz);
    float3 axizY = normalize(meshToWorld[1].xyz);
    float3 axizZ = normalize(meshToWorld[2].xyz);
    vsOutput.o_Normal = vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ;

    vsOutput.o_BaseColor = instInput.a_BaseColor;
    vsOutput.o_AddColor = instInput.a_AddColor;

    return vsOutput;
}
