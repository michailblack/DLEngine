#include "GlobalCB.hlsli"

cbuffer MeshInstance : register(b2)
{
    float4x4 c_MeshToModel;
    float4x4 c_ModelToMesh;
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
    float4x4 a_ModelToWorld : TRANSFORM;
    float3 a_BaseColor      : BASE_COLOR;
    float3 a_AddColor       : ADD_COLOR;
};

struct VertexOutput
{
    float4 v_Position  : SV_POSITION;
    float3 v_WorldPos  : WORLD_POS;
    float3 v_Normal    : NORMAL;
    float3 v_BaseColor : BASE_COLOR;
    float3 v_AddColor  : ADD_COLOR;
};

VertexOutput mainVS(VertexInput vsInput, InstanceInput instInput)
{
    VertexOutput vsOutput;

    float4x4 meshToWorld = mul(c_MeshToModel, instInput.a_ModelToWorld);
    float4 vertexPos = mul(float4(vsInput.a_Position, 1.0), meshToWorld);

    vsOutput.v_WorldPos = vertexPos.xyz;
    vsOutput.v_Position = mul(vertexPos, c_ViewProjection);

    float3 axisX = normalize(meshToWorld[0].xyz);
    float3 axizY = normalize(meshToWorld[1].xyz);
    float3 axizZ = normalize(meshToWorld[2].xyz);
    vsOutput.v_Normal = vsInput.a_Normal.x * axisX + vsInput.a_Normal.y * axizY + vsInput.a_Normal.z * axizZ;

    vsOutput.v_BaseColor = instInput.a_BaseColor;
    vsOutput.v_AddColor = instInput.a_AddColor;

    return vsOutput;
}

struct HullPatchOutput
{
	float EdgeTessFactor[3]	: SV_TessFactor;
	float InsideTessFactor	: SV_InsideTessFactor;
};

static const uint NUM_CONTROL_POINTS = 3;
static const float MAX_TESS_FACTOR = 64.0;
static const float MIN_TESS_FACTOR = 1.0;
static const float SCALING_CONSTANT = 0.5;

float CalcTessFactor(float edgeLength)
{
    return min(MAX_TESS_FACTOR, MIN_TESS_FACTOR + SCALING_CONSTANT * edgeLength);
}

HullPatchOutput CalcHSPatchConstants(
    InputPatch<VertexOutput, NUM_CONTROL_POINTS> inputPatch,
    uint patchID : SV_PrimitiveID
)
{
	HullPatchOutput patchOutput;

	float AB = length(inputPatch[1].v_WorldPos - inputPatch[0].v_WorldPos);
    float BC = length(inputPatch[2].v_WorldPos - inputPatch[1].v_WorldPos);
    float CA = length(inputPatch[0].v_WorldPos - inputPatch[2].v_WorldPos);
    float maxEdgeLength = max(AB, max(BC, CA));

	patchOutput.EdgeTessFactor[0] = CalcTessFactor(AB);
	patchOutput.EdgeTessFactor[1] = CalcTessFactor(BC);
    patchOutput.EdgeTessFactor[2] = CalcTessFactor(CA);
    patchOutput.InsideTessFactor  = CalcTessFactor(maxEdgeLength);
    
	return patchOutput;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(MAX_TESS_FACTOR)]
VertexOutput mainHS(
    InputPatch<VertexOutput, NUM_CONTROL_POINTS> inputPatch,
    uint pointID : SV_OutputControlPointID,
    uint patchID : SV_PrimitiveID
)
{
	return inputPatch[pointID];
}

[domain("tri")]
VertexOutput mainDS(
    HullPatchOutput patchConstantData,
    float3 bary : SV_DomainLocation,
    const OutputPatch<VertexOutput, NUM_CONTROL_POINTS> patch
)
{
	VertexOutput dsOutput;

    dsOutput.v_Position  = bary.x * patch[0].v_Position  + bary.y * patch[1].v_Position  + bary.z * patch[2].v_Position;
    dsOutput.v_WorldPos  = bary.x * patch[0].v_WorldPos  + bary.y * patch[1].v_WorldPos  + bary.z * patch[2].v_WorldPos;
    dsOutput.v_Normal    = bary.x * patch[0].v_Normal    + bary.y * patch[1].v_Normal    + bary.z * patch[2].v_Normal;
    dsOutput.v_BaseColor = bary.x * patch[0].v_BaseColor + bary.y * patch[1].v_BaseColor + bary.z * patch[2].v_BaseColor;
    dsOutput.v_AddColor  = bary.x * patch[0].v_AddColor  + bary.y * patch[1].v_AddColor  + bary.z * patch[2].v_AddColor;

	return dsOutput;
}

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

static const float BASE_WAVE_INTERVAL = 0.8;
static const float BASE_WAVE_SPEED = 0.25;
static const float BASE_WAVE_THICKNESS = 0.05;

static const float ADD_WAVE_INTERVAL = 10;
static const float ADD_WAVE_SPEED = 2;
static const float ADD_WAVE_THICKNESS = 0.2;

float3 vertexDistortion(float3 pos, float3 normal)
{
	float3 offset = 0.0;
	offset += normal * 0.025 * wave(pos, BASE_WAVE_INTERVAL, BASE_WAVE_SPEED, BASE_WAVE_THICKNESS, true);
	offset += normal * 0.05 * wave(pos, ADD_WAVE_INTERVAL, ADD_WAVE_SPEED, ADD_WAVE_THICKNESS, false);
	return offset;
}

[maxvertexcount(3)]
void mainGS(triangle VertexOutput input[3], inout TriangleStream<VertexOutput> output)
{
	float3 edge1 = input[1].v_WorldPos - input[0].v_WorldPos;
	float3 edge2 = input[2].v_WorldPos - input[0].v_WorldPos;
	float3 triangleNormal = normalize(cross(edge1, edge2));
	
    float3 triangleCenter = (input[0].v_WorldPos + input[1].v_WorldPos + input[2].v_WorldPos) / 3.0;
    float3 offset = vertexDistortion(triangleCenter, triangleNormal);
    
	for (uint i = 0; i < 3; i++)
	{
		VertexOutput gsOutput;
        gsOutput = input[i];
		
        gsOutput.v_WorldPos = input[i].v_WorldPos + offset;
        gsOutput.v_Position = mul(float4(gsOutput.v_WorldPos, 1.0), c_ViewProjection);
		        
		output.Append(gsOutput);
	}
}

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

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    return float4(colorDistortion(psInput.v_WorldPos, psInput.v_Normal, psInput.v_BaseColor, psInput.v_AddColor), 1.0);
}
