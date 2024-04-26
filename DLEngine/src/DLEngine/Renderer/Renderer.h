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

    static Math::Ray GetRay(uint32_t mouseX, uint32_t mouseY);

private:
    static Environment CalculateEnvironmentContribution(const Math::IntersectInfo& intersectionInfo, const Environment& environment);
    static bool PointIsOccluded(const Math::Ray& ray, const float lightSourceT);
};
