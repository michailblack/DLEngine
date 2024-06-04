#include "DLEngine/Core/EntryPoint.h"

#include "Sandbox/WorldLayer.h"

namespace DLEngine
{
    class SandboxApp
        : public Application
    {
    public:
        SandboxApp(const ApplicationSpecification& spec)
            : Application(spec)
        {
            PushLayer(new WorldLayer{});
        }
    };

    Application* CreateApplication(std::wstring_view cmdLine)
    {
        ApplicationSpecification spec;
        spec.WndTitle = L"Sandbox";
        spec.WorkingDir = cmdLine.data();
        spec.WndWidth = 800;
        spec.WndHeight = 600;
        return new SandboxApp(spec);
    }
}
