#include "DLEngine/Core/EntryPoint.h"

#include "Sandbox/WorldLayer.h"

class SandboxApp
    : public Application
{
public:
    SandboxApp(const ApplicationSpecification& spec)
        : Application(spec)
    {
        PushLayer(new WorldLayer {});
    }
};

Application* CreateApplication()
{
    ApplicationSpecification spec;
    spec.WndTitle  = L"Sandbox";
    spec.WndWidth  = 600;
    spec.WndHeight = 450;
    return new SandboxApp(spec);
}
