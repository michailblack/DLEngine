#include "Include/Lighting.hlsli"

struct InstanceInput
{
    float3 a_WorldPos          : PARTICLE_WORLD_POS;
    float3 a_TintColor         : PARTICLE_TINT_COLOR;
    float2 a_InitialSize       : PARTICLE_INITIAL_SIZE;
    float2 a_EndSize           : PARTICLE_END_SIZE;
    float  a_EmissionIntensity : PARTICLE_EMISSION_INTENSITY;
    float  a_LifetimeMS        : PARTICLE_LIFETIME_MS;
    float  a_LifetimePassedMS  : PARTICLE_LIFETIME_PASSED_MS;
    float  a_Rotation          : PARTICLE_ROTATION;
};

struct VertexOutput
{
    float4   v_Position          : SV_POSITION;
    float4   v_ClipSpacePos      : CLIP_SPACE_POS;
    float3x3 v_Basis             : PARTICLE_BASIS;
    float3   v_WorldPos          : WORLD_POS;
    float3   v_Normal            : NORMAL;
    float3   v_TintColor         : TINT_COLOR;
    float2   v_TexCoords         : TEXCOORDS;
    float    v_EmissionIntensity : EMISSION_INTENSITY;
    float    v_LifetimeMS        : LIFETIME_MS;
    float    v_LifetimePassedMS  : LIFETIME_PASSED_MS;
    float    v_FrameFraction     : FRAME_FRACTION; // goes from 0.0 to 1.0 between two sequential frames
    uint     v_FrameIndex        : FRAME_INDEX;
};

cbuffer TextureAtlasData : register(b4)
{
    float2 c_TextureAtlasSize;
    float2 c_TextureAtlasTileSize;
    uint   c_TilesCountX;
    uint   c_TilesCountY;
};

static const float2 BillboardVertexDirections[4] =
{
    normalize(float2(-1.0, -1.0)),
    normalize(float2(-1.0,  1.0)),
    normalize(float2( 1.0,  1.0)),
    normalize(float2( 1.0, -1.0))
};

static const float2 BillboardTexCoords[4] =
{
    float2(0.0, 1.0),
    float2(0.0, 0.0),
    float2(1.0, 0.0),
    float2(1.0, 1.0)
};

Texture2D<float3> t_RLU_Atlas  : register(t16);
Texture2D<float3> t_DBF_Atlas  : register(t17);
Texture2D<float4> t_EMVA_Atlas : register(t18);
Texture2D<float>  t_SceneDepth : register(t19);

static const float MotionVectorScale = 0.0015;
static const float SmokeThickness    = 0.001;

float2 LocalUVToTextureAtlasUV(float2 uv, uint frameIndex)
{
    const float2 tileSizeUVSpace = 1.0 / float2(c_TilesCountX, c_TilesCountY);
    const float2 tilePos = float2(frameIndex % c_TilesCountX, frameIndex / c_TilesCountX);
    return tilePos * tileSizeUVSpace + uv * tileSizeUVSpace;
}

VertexOutput mainVS(InstanceInput instInput, uint vertexID : SV_VertexID)
{
    const float timePassedPercentage = instInput.a_LifetimePassedMS / instInput.a_LifetimeMS;
    const float2 billboardSize = instInput.a_InitialSize + (instInput.a_EndSize - instInput.a_InitialSize) * smoothstep(0.0, 1.0, timePassedPercentage);
    
    const float2 billboardVertexDelta = billboardSize * BillboardVertexDirections[vertexID % 4] * 0.5;
    
    const float3 cameraRight   = normalize(c_InvView[0].xyz);
    const float3 cameraUp      = normalize(c_InvView[1].xyz);
    const float3 cameraForward = normalize(c_InvView[2].xyz);
    
    float3 billboardVertexWorldPos = instInput.a_WorldPos + cameraRight * billboardVertexDelta.x + cameraUp * billboardVertexDelta.y;
    billboardVertexWorldPos = RodriguesRotation(billboardVertexWorldPos, instInput.a_WorldPos, cameraForward, smoothstep(0.0, instInput.a_Rotation, timePassedPercentage));

    VertexOutput vsOutput;
    vsOutput.v_Position  = mul(float4(billboardVertexWorldPos, 1.0), c_ViewProjection);
    vsOutput.v_ClipSpacePos = vsOutput.v_Position;
    
    vsOutput.v_Basis[0]  = cameraRight;
    vsOutput.v_Basis[1]  = cameraUp;
    vsOutput.v_Basis[2]  = cameraForward;
    
    vsOutput.v_WorldPos  = billboardVertexWorldPos;
    vsOutput.v_Normal    = -cameraForward;
    vsOutput.v_TintColor = instInput.a_TintColor;
    vsOutput.v_TexCoords = BillboardTexCoords[vertexID % 4];
    vsOutput.v_EmissionIntensity = instInput.a_EmissionIntensity;
    vsOutput.v_LifetimeMS = instInput.a_LifetimeMS;
    vsOutput.v_LifetimePassedMS = instInput.a_LifetimePassedMS;
    
    const float framesCount = float(c_TilesCountX * c_TilesCountY);
    
    const float frameTime = instInput.a_LifetimeMS / framesCount;
    vsOutput.v_FrameFraction = fmod(instInput.a_LifetimePassedMS, frameTime) / frameTime;
    vsOutput.v_FrameIndex = uint(timePassedPercentage * framesCount);
    
    return vsOutput;
}

float LinearizeReversedDepth(float depth)
{
    return (c_zNear * c_zFar) / lerp(c_zFar, c_zNear, depth);
}

void SurfaceAttenuation(inout float alpha, float linearDepth, float surfaceLinearDepth)
{
    const float distanceToSurface = surfaceLinearDepth - linearDepth;
    alpha = lerp(0.0, alpha, saturate(distanceToSurface / SmokeThickness));
}

void NearPlaneAttenuation(inout float alpha, float linearDepth, float frustumThresholdDistancePercentage)
{
    const float thresholdDistance = (c_zNear - c_zFar) * frustumThresholdDistancePercentage;
    const float distanceToNearPlane = linearDepth - c_zFar;
    const float attenuationFactor = saturate(distanceToNearPlane / thresholdDistance);
    
    alpha = lerp(0.0, alpha, attenuationFactor);
}

void RevealAttenuation(inout float alpha, float spawnLifetimePercentage, float despawnLifetimePercentage, float lifetimePassedPercentage)
{
    float attenuationFactor = saturate(lifetimePassedPercentage / spawnLifetimePercentage);
    alpha = lerp(0.0, alpha, attenuationFactor);
    
    attenuationFactor = saturate((1.0 - lifetimePassedPercentage) / (1.0 - despawnLifetimePercentage));
    alpha = lerp(alpha, 0.0, attenuationFactor);
}

float3 SixWayLightMapContribution(float3 lightDir, float3 lightRadiance, float3 lightmapRLU, float3 lightmapDBF, float3x3 basis)
{
    float3 lightContribution = float3(0.0, 0.0, 0.0);
    
    const float3 particleRight = basis[0];
    const float3 particleLeft  = -particleRight;
    lightContribution += lightRadiance * lightmapRLU.r * max(dot(lightDir, particleRight), Epsilon);
    lightContribution += lightRadiance * lightmapRLU.g * max(dot(lightDir, particleLeft) , Epsilon);
    
    const float3 particleUp   = basis[1];
    const float3 particleDown = -particleUp;
    lightContribution += lightRadiance * lightmapRLU.b * max(dot(lightDir, particleUp)  , Epsilon);
    lightContribution += lightRadiance * lightmapDBF.r * max(dot(lightDir, particleDown), Epsilon);
    
    const float3 particleBack  = basis[2];
    const float3 particleFront = -particleBack;
    lightContribution += lightRadiance * lightmapDBF.g * max(dot(lightDir, particleBack) , Epsilon);
    lightContribution += lightRadiance * lightmapDBF.b * max(dot(lightDir, particleFront), Epsilon);
    
    return lightContribution;
}

float4 mainPS(VertexOutput psInput) : SV_TARGET
{
    const float2 uvThisFrame = LocalUVToTextureAtlasUV(psInput.v_TexCoords, psInput.v_FrameIndex);
    const float2 uvNextFrame = LocalUVToTextureAtlasUV(psInput.v_TexCoords, (psInput.v_FrameIndex + 1));
    
    // ----------- sample motion-vectors -----------
    
    float2 mv0 = 2.0 * t_EMVA_Atlas.Sample(s_TrilinearWrap, uvThisFrame).gb - 1.0;
    float2 mv1 = 2.0 * t_EMVA_Atlas.Sample(s_TrilinearWrap, uvNextFrame).gb - 1.0;
    
    // need to flip motion-vector Y specifically for the smoke textures:
    mv0.y = -mv0.y;
    mv1.y = -mv1.y;
    
    // ----------- UV flowing along motion-vectors -----------
    
    float2 uv0 = uvThisFrame;
    uv0 -= mv0 * MotionVectorScale * psInput.v_FrameFraction;
    
    float2 uv1 = uvNextFrame;
    uv1 -= mv1 * MotionVectorScale * (psInput.v_FrameFraction - 1.0);
    
    // ----------- sample textures -----------
    
    float2 emissionAlpha0 = t_EMVA_Atlas.Sample(s_TrilinearWrap, uv0).ra;
    float2 emissionAlpha1 = t_EMVA_Atlas.Sample(s_TrilinearWrap, uv1).ra;
    
    // .x - right, .y - left, .z - up
    float3 lightmapRLU0 = t_RLU_Atlas.Sample(s_TrilinearWrap, uv0).rgb;
    float3 lightmapRLU1 = t_RLU_Atlas.Sample(s_TrilinearWrap, uv1).rgb;
    
    // .x - down, .y - back, .z - front
    float3 lightmapDBF0 = t_DBF_Atlas.Sample(s_TrilinearWrap, uv0).rgb;
    float3 lightmapDBF1 = t_DBF_Atlas.Sample(s_TrilinearWrap, uv1).rgb;
    
    // ----------- lerp values -----------
    
    float2 emissionAlpha = lerp(emissionAlpha0, emissionAlpha1, psInput.v_FrameFraction);
    float3 lightmapRLU = lerp(lightmapRLU0, lightmapRLU1, psInput.v_FrameFraction);
    float3 lightmapDBF = lerp(lightmapDBF0, lightmapDBF1, psInput.v_FrameFraction);
    
    // ----------- calculate light contribution -----------
    float3x3 particleBasis = psInput.v_Basis;
    particleBasis[0] = normalize(particleBasis[0]);
    particleBasis[1] = normalize(particleBasis[1]);
    particleBasis[2] = normalize(particleBasis[2]);
    
    const float3 particleNormal = normalize(psInput.v_Normal);
    
    float3 lightContribution = float3(0.0, 0.0, 0.0);
    for (uint directionalLightIndex = 0; directionalLightIndex < c_DirectionalLightsCount; ++directionalLightIndex)
    {
        const DirectionalLight directionalLight = t_DirectionalLights[directionalLightIndex];
        const float3 lightDir = normalize(-directionalLight.Direction);
        
        float3 directionalLightContribution = SixWayLightMapContribution(lightDir, directionalLight.Radiance * directionalLight.SolidAngle, lightmapRLU, lightmapDBF, particleBasis);
        
        if (c_UseDirectionalShadows)
            directionalLightContribution *= VisibilityForDirectionalLight(directionalLightIndex, psInput.v_WorldPos, particleNormal);
        
        lightContribution += directionalLightContribution;
    }
    
    for (uint pointLightIndex = 0; pointLightIndex < c_PointLightsCount; ++pointLightIndex)
    {
        const PointLight pointLight = t_PointLights[pointLightIndex];
        
        const float3 fragmentToLight = pointLight.Position - psInput.v_WorldPos;
        
        const float sphereDist = max(length(fragmentToLight), pointLight.Radius);
        const float solidAngle = CalculateSphereSolidAngle(pointLight.Radius, sphereDist);
        
        const float3 lightRadiance = solidAngle * pointLight.Radiance;
        
        float3 pointLightContribution = SixWayLightMapContribution(normalize(fragmentToLight), lightRadiance, lightmapRLU, lightmapDBF, particleBasis);
        
        if (c_UseOmnidirectionalShadows)
            pointLightContribution *= VisibilityForPointLight(pointLightIndex, psInput.v_WorldPos, particleNormal);
        
        lightContribution += pointLightContribution;
    }
    
    for (uint spotLightIndex = 0; spotLightIndex < c_SpotLightsCount; ++spotLightIndex)
    {
        const SpotLight spotLight = t_SpotLights[spotLightIndex];
        
        const float3 fragmentToLight = spotLight.Position - psInput.v_WorldPos;
        const float3 lightDir = normalize(fragmentToLight);
        
        const float sphereDist = max(length(fragmentToLight), spotLight.Radius);
        const float solidAngle = CalculateSphereSolidAngle(spotLight.Radius, sphereDist);
        
        const float3 lightRadiance = solidAngle * spotLight.Radiance * CalclulateSpotLightCutoffIntensity(spotLight, lightDir);
        
        float3 spotLightContribution = SixWayLightMapContribution(lightDir, lightRadiance, lightmapRLU, lightmapDBF, particleBasis);
        
        if (c_UseSpotShadows)
            spotLightContribution *= VisibilityForSpotLight(spotLightIndex, psInput.v_WorldPos, particleNormal);
        
        lightContribution += spotLightContribution;
    }
    
    // ----------- calculate final color -----------
    
    float3 resultColor = lightContribution;
    resultColor = lerp(resultColor, resultColor * psInput.v_EmissionIntensity, emissionAlpha.r);
    resultColor *= psInput.v_TintColor;
    
    if (c_UseIBL)
    {
        const float3 diffuseIrradianceIBL = t_EnvironmentIrradiance.Sample(s_Anisotropic8Clamp, particleNormal).rgb;
        const float3 diffuseReflection = diffuseIrradianceIBL * psInput.v_TintColor;
        
        resultColor += diffuseReflection;
    }
    
    // ----------- calculate alpha -----------
    
    const float3 projectedCoords = psInput.v_ClipSpacePos.xyz / psInput.v_ClipSpacePos.w;
    
    const float particleDepth = LinearizeReversedDepth(projectedCoords.z);
    
    float2 screenUV = projectedCoords.xy * 0.5 + 0.5;
    screenUV.y = 1.0 - screenUV.y;
    const float sceneDepth = LinearizeReversedDepth(t_SceneDepth.Sample(s_TrilinearClamp, screenUV).r);
    
    float alpha = 1.0;
    SurfaceAttenuation(alpha, particleDepth, sceneDepth);
    NearPlaneAttenuation(alpha, particleDepth, 0.75);
    RevealAttenuation(alpha, 0.1, 0.3, psInput.v_LifetimePassedMS / psInput.v_LifetimeMS);
    alpha = lerp(0.0, alpha, emissionAlpha.g);
    
    return float4(resultColor, alpha);
}