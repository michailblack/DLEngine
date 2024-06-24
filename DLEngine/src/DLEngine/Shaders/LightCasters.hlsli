struct DirectionalLight
{
    float3 Direction;
    float _padding;
    float3 Luminance;
    uint TransformIndex;
};

struct PointLight
{
    float3 Position;
    float Linear;
    float3 Luminance;
    float Quadratic;
    uint TransformIndex;
    float3 _padding;
};

struct SpotLight
{
    float3 Position;
    float Linear;
    float3 Direction;
    float Quadratic;
    float3 Luminance;
    float InnerCutoffCos;
    float OuterCutoffCos;
    uint TransformIndex;
    float2 _padding;
};