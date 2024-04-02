#include "DLEngine/Core/EntryPoint.h"

class SandboxApp : public Application
{
public:
    SandboxApp (const ApplicationSpecification& specification)
        : Application(specification)
    {}
};

Application* CreateApplication()
{
    return new SandboxApp({ 800, 600, L"Simple Ray Casting" });
}