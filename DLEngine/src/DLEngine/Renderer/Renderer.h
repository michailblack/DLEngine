#pragma once
#include "DLEngine/Renderer/Camera.h"
#include "DLEngine/Renderer/Entity.h"

class Renderer
{
public:
    static void Init();

    static void BeginScene(const Camera& camera, const Ref<Environment>& environment);
    static void EndScene();

    static void Submit(const Ref<SphereInstance>& sphere);
    static void Submit(const Ref<PlaneInstance>& plane);
    static void Submit(const Ref<MeshInstance>& cube);

    static void OnResize(uint32_t width, uint32_t height);

private:
    static void RenderPerThread(uint32_t startHeight, uint32_t height, const Math::Vec4& BL, const Math::Vec4& Right, const Math::Vec4& Up);

    static Environment CalculateEnvironmentContribution(const Math::IntersectInfo& intersectionInfo, const Environment& environment);
    static bool PointIsOccluded(const Math::Ray& ray, const float lightSourceT);
};
