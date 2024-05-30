#include "dlpch.h"
#include "Engine.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/DXGIInfoQueue.h"
#include "DLEngine/DirectX/Shaders.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"
#include "DLEngine/Systems/Renderer/Renderer.h"

namespace DLEngine
{
    namespace
    {
        void InitConsole()
        {
            AllocConsole();
            FILE* dummy;
            freopen_s(&dummy, "conout$", "w", stdout);
            freopen_s(&dummy, "conout$", "w", stderr);
        }
    }

    void Engine::PreInit()
    {
        if (!DirectX::XMVerifyCPUSupport())
            throw std::runtime_error{ "DirectXMath Library does not support the given platform" };

        InitConsole();

        Log::Init();
        DXGIInfoQueue::Get().Init();
        D3D::Init();
    }

    void Engine::Init()
    {
        Renderer::Init();
        MeshSystem::Get().Init();
    }

    void Engine::Deinit()
    {

    }
}
