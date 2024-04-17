#pragma once
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Renderer/Camera.h"
#include "DLEngine/Renderer/Mesh.h"

struct SphereInstance
{
    Math::Sphere Sphere;
};

struct PlaneInstance
{
    Math::Plane Plane;
};

struct MeshInstance
{
    Math::Mat4x4 Transform;
    Math::Mat4x4 InvTransform;
};

class Renderer
{
public:
    static void Init();

    static void BeginScene(const Camera& camera);
    static void EndScene();

    static void Submit(const Ref<SphereInstance>& sphere);
    static void Submit(const Ref<PlaneInstance>& plane);
    static void Submit(const Ref<MeshInstance>& cube);

    static void OnResize(uint32_t width, uint32_t height);

private:
    static void RenderPerThread(uint32_t startHeight, uint32_t height, const Math::Vec4& BL, const Math::Vec4& Right, const Math::Vec4& Up);
};
