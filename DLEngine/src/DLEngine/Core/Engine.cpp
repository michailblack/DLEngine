#include "dlpch.h"
#include "Engine.h"

#include "DLEngine/Core/Filesystem.h"

#include "DLEngine/DirectX/D3D.h"
#include "DLEngine/DirectX/DXGIInfoQueue.h"
#include "DLEngine/DirectX/Shaders.h"

#include "DLEngine/Renderer/Renderer.h"
#include "DLEngine/Renderer/TextureManager.h"

#include "DLEngine/Systems/Light/LightSystem.h"

#include "DLEngine/Systems/Mesh/MeshSystem.h"
#include "DLEngine/Systems/Mesh/ModelManager.h"

#include "DLEngine/Systems/Transform/TransformSystem.h"

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

    void Engine::Init()
    {
        if (!DirectX::XMVerifyCPUSupport())
            throw std::runtime_error{ "DirectXMath Library does not support the given platform" };

        InitConsole();

        Log::Init();
        DXGIInfoQueue::Get().Init();
        D3D::Init();
        Filesystem::Init();
        ShaderIncludeHandler::Get()->Init();
        Renderer::Init();
        ModelManager::Init();
        TextureManager::Init();
        MeshSystem::Get().Init();
        TransformSystem::Init();
        LightSystem::Init();

        DL_LOG_INFO("Fibonacci hemisphere point generation check...");

        const uint32_t numPoints{ 1024u };
        const Math::Vec3 normal{ 0.0f, 0.0f, 1.0f };
        const auto hemispherePoints{ Math::GenerateFibonacciHemispherePoints(numPoints) };
        float integrationResult{ 0.0f };
        for (const auto& point : hemispherePoints)
            integrationResult += Math::Dot(point, normal);
        integrationResult *= 2.0f * Math::Numeric::Pi / numPoints;

        DL_LOG_INFO("Integration result for {} points: {}", numPoints, integrationResult);
    }

    void Engine::Deinit()
    {

    }
}
