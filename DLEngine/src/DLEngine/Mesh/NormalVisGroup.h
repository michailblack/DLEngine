#pragma once
#include "DLEngine/Math/Mat4x4.h"

#include "DLEngine/Mesh/Model.h"
#include "DLEngine/Mesh/ShaderGroup.h"

namespace DLEngine
{
    struct NormalVisGroupMaterial
    {
        bool operator==(const NormalVisGroupMaterial&) const noexcept { return true; }
    };

    struct NormalVisGroupInstance
    {
        Math::Mat4x4 Transform;
    };

    class NormalVisGroup
        : public ShaderGroup<NormalVisGroupMaterial, NormalVisGroupInstance>
    {
    public:
        NormalVisGroup();
        ~NormalVisGroup() = default;

        NormalVisGroup(const NormalVisGroup&) = delete;
        NormalVisGroup(NormalVisGroup&&) = delete;
        NormalVisGroup& operator=(const NormalVisGroup&) = delete;
        NormalVisGroup& operator=(NormalVisGroup&&) = delete;

        Ref<MeshDragger> CreateMeshDragger(const Math::Ray& ray, const Math::Vec3& cameraForward, const IShaderGroup::IntersectInfo& intersectInfo) override;

    protected:
        void UpdateAndSetPerDrawBuffer(uint32_t modelIndex, uint32_t meshIndex, uint32_t instanceIndex) const override;
        bool IntersectsInstance(const Math::Ray& ray, IShaderGroup::IntersectInfo& outIntersectInfo) const override;

    private:
        struct PerDraw
        {
            Math::Mat4x4 MeshToModel;
            Math::Mat4x4 ModelToMesh;
        };

    private:
        Ref<ConstantBuffer<PerDraw>> m_PerDrawConstantBuffer;
    };
}

namespace DLEngine
{
    class NormalVisGroupDragger
        : public MeshDragger
    {
    public:
        NormalVisGroupDragger(NormalVisGroupInstance& instance, Math::Vec3 startDraggingPoint, float distanceToDraggingPlane)
            : MeshDragger(startDraggingPoint, distanceToDraggingPlane)
            , m_Instance(instance)
        {}
        ~NormalVisGroupDragger() override = default;

        void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) override;

    private:
        NormalVisGroupInstance& m_Instance;
    };
}
