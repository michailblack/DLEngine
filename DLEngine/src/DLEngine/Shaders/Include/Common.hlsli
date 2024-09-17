#ifndef _COMMON_HLSLI_
#define _COMMON_HLSLI_

static const float PI = 3.14159265359;
static const float Epsilon = 1e-5;

static const float3 CubeFaceNormals[6] =
{
    float3( 1.0,  0.0,  0.0), // +X
    float3(-1.0,  0.0,  0.0), // -X
    float3( 0.0,  1.0,  0.0), // +Y
    float3( 0.0, -1.0,  0.0), // -Y
    float3( 0.0,  0.0,  1.0), // +Z
    float3( 0.0,  0.0, -1.0)  // -Z
};

// n is a normalized vector of a new basis
void BranchelssONB(float3 n, out float3 b1, out float3 b2)
{
    // To avoid division by zero
    float s = float(sign(n.z)) + Epsilon;
    s = float(sign(s));
    
    const float a = -1.0 / (s + n.z);
    const float b = n.x * n.y * a;

    b1 = float3(1.0 + s * n.x * n.x * a, s * b, -s * n.x);
    b2 = float3(b, s + n.y * n.y * a, -n.y);
}

float3x3 BranchlessONB(float3 n)
{
    float3x3 rotation;
    BranchelssONB(n, rotation[0], rotation[1]);
    rotation[2] = n;
    return rotation;
}

// n is a normalized vector of a new basis
float3 RotateToNewBasis(float3 v, float3 n)
{
    return mul(v, BranchlessONB(n));
}

uint CubeFaceIndex(float3 unitDir)
{    
    const float3 absDir = abs(unitDir);
    const float maxComponent = max(max(absDir.x, absDir.y), absDir.z);
    const uint maxIndex = (absDir.x == maxComponent) ? 0 : (absDir.y == maxComponent) ? 2 : 4;
    return maxIndex + (asuint(unitDir[maxIndex >> 1]) >> 31);
}

float3x3 ConstructNormalMatrix(float4x4 transform)
{
    const float3 axisX = normalize(transform[0].xyz);
    const float3 axizY = normalize(transform[1].xyz);
    const float3 axizZ = normalize(transform[2].xyz);
    
    return float3x3(axisX, axizY, axizZ);
}

float3 RandomConeDirection(uint index, uint samplesCount, float coneHalfAngleCos)
{
    const float z = lerp(coneHalfAngleCos, 1.0, float(index) / float(samplesCount - 1));
    const float r = sqrt(1.0 - z * z);
    
    const float phi = 2.0 * PI * lerp(0.0, 1.0, float(index) / float(samplesCount));
    
    return float3(r * cos(phi), r * sin(phi), z);
}

float3 RodriguesRotation(float3 v, float3 normalizedAxis, float theta)
{
    const float cosTheta = cos(theta);
    const float sinTheta = sin(theta);
    
    return v * cosTheta + cross(normalizedAxis, v) * sinTheta + normalizedAxis * dot(normalizedAxis, v) * (1.0 - cosTheta);
}

float3 RodriguesRotation(float3 v, float3 origin, float3 normalizedAxis, float theta)
{
    const float3 relativePos = v - origin;
    const float3 rotatedRelativePos = RodriguesRotation(relativePos, normalizedAxis, theta);
    return origin + rotatedRelativePos;
}

// May return direction pointing beneath surface horizon (dot(N, dir) < 0), use clampDirToHorizon to fix it.
// sphereCos is cosine of the light source angular halfsize (2D angle, not solid angle).
// sphereRelPos is position of a sphere relative to surface:
// 'sphereDir == normalize(sphereRelPos)' and 'sphereDir * sphereDist == sphereRelPos'
float3 ApproximateClosestSphereDir(float3 reflectionDir, float sphereCos, float3 sphereRelPos, float3 sphereDir, float sphereDist, float sphereRadius)
{
    float3 closestSphereDir = float3(0.0, 0.0, 0.0);
    const float RoS = dot(reflectionDir, sphereDir);

    if (RoS >= sphereCos)
    {
        closestSphereDir = reflectionDir;
    }
    else if (RoS < 0.0)
    {
        closestSphereDir = sphereDir;
    }
    else
    {
        const float3 closestPointDir = normalize(reflectionDir * sphereDist * RoS - sphereRelPos);
        closestSphereDir = normalize(sphereRelPos + sphereRadius * closestPointDir);
    }

    return closestSphereDir;
}

// Input dir and NoD is L and NoL in a case of lighting computation 
void ClampDirToHorizon(inout float3 dir, inout float NoD, float3 normal, float minNoD)
{
    if (NoD < minNoD)
    {
        dir = normalize(dir + (minNoD - NoD) * normal);
        NoD = minNoD;
    }
}

#endif