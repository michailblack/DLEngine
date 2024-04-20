#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Layer.h"

#include "DLEngine/Core/Events/ApplicationEvent.h"

#include "DLEngine/Renderer/CameraController.h"
#include "DLEngine/Renderer/Entity.h"

class WorldLayer
    : public Layer
{
public:
    WorldLayer();
    ~WorldLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float dt) override;
    void OnEvent(Event& e) override;

private:
    Scope<IDragger> FindDragger(const Math::Ray& ray) const;

private:
    CameraController m_CameraController;

    std::vector<Ref<SphereInstance>> m_Spheres;
    std::vector<Ref<PlaneInstance>> m_Planes;
    std::vector<Ref<MeshInstance>> m_Cubes;

    Ref<Environment> m_Environment;
};

