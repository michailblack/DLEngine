struct DirectionalLight
{
    float3 Direction;
    float3 Radiance;
    float SolidAngle;
    uint TransformIndex;
};

struct PointLight
{
    float3 Position;
    float3 Radiance;
    float Radius;
    uint TransformIndex;
};

struct SpotLight
{
    float3 Position;
    float3 Direction;
    float3 Radiance;
    float Radius;
    float InnerCutoffCos;
    float OuterCutoffCos;
    uint TransformIndex;
};