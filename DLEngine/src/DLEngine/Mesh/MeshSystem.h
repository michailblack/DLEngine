#pragma once
#include <any>

#include "DLEngine/Core/Base.h"

#include "DLEngine/Mesh/IDragger.h"
#include "DLEngine/Mesh/Model.h"

namespace DLEngine
{
    class MeshSystem
    {
    public:
        static void Init();

        static void Render();

        static void AddModel(const Ref<Model>& model, const std::any& material, const std::any& instance);

        static bool Intersects(const Math::Ray& ray, const Math::Vec3& cameraForward, Ref<IDragger>& outMeshDragger);
    };
}
