struct DirectionalLight
{
    float3 Direction;
    float SolidAngle;
    float3 Luminance;
    uint TransformIndex;
};

struct PointLight
{
    float3 Position;
    float Radius;
    float3 Luminance;
    uint TransformIndex;
};

struct SpotLight
{
    float3 Position;
    float InnerCutoffCos;
    float3 Direction;
    float OuterCutoffCos;
    float3 Luminance;
    float Radius;
    uint TransformIndex;
    float3 _padding;
};