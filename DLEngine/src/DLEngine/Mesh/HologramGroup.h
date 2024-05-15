#pragma once
#include "DLEngine/Math/Mat4x4.h"
#include "DLengine/Math/Vec3.h"

#include "DLEngine/Mesh/Model.h"
#include "DLEngine/Mesh/ShaderGroup.h"

namespace DLEngine
{
    struct HologramGroupMaterial
    {
        bool operator==(const HologramGroupMaterial&) const noexcept { return true; }
    };

    struct HologramGroupInstance
    {
        Math::Mat4x4 Transform;
        Math::Vec3 BaseColor;
        Math::Vec3 AdditionalColor;
    };

    class HologramGroup
        : public ShaderGroup<HologramGroupMaterial, HologramGroupInstance>
    {
    public:
        HologramGroup();
        ~HologramGroup() = default;

        HologramGroup(const HologramGroup&) = delete;
        HologramGroup(HologramGroup&&) = delete;
        HologramGroup& operator=(const HologramGroup&) = delete;
        HologramGroup& operator=(HologramGroup&&) = delete;

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
    class HologramGroupDragger
        : public MeshDragger
    {
    public:
        HologramGroupDragger(HologramGroupInstance& instance, Math::Vec3 startDraggingPoint, float distanceToDraggingPlane)
            : MeshDragger(startDraggingPoint, distanceToDraggingPlane)
            , m_Instance(instance)
        {}
        ~HologramGroupDragger() override = default;

        void Drag(const Math::Plane& nearPlane, const Math::Ray& endRay) override;

    private:
        HologramGroupInstance& m_Instance;
    };
}
