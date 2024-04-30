#pragma once
#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/DLException.h"
#include "DLEngine/Core/DXGIInfoQueueManager.h"

#include "DLEngine/DirectX/D3D.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    try
    {
        if (!DirectX::XMVerifyCPUSupport())
            throw std::runtime_error("DirectXMath Library does not support the given platform");

        D3D::Get().Init();
        DXGIInfoQueueManager::Get().Init();

        const Scope<Application> app { CreateApplication() };
        app->Run();

        return 0;
    }
    catch (const DLException& e)
    {
        MessageBoxExA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION, 0);
    }
    catch (const std::exception& e)
    {
        MessageBoxExA(nullptr, e.what(), "Standard exception", MB_OK | MB_ICONEXCLAMATION, 0);
    }
    catch (...)
    {
        MessageBoxExA(nullptr, "Unknown exception", "Unknown exception", MB_OK | MB_ICONEXCLAMATION, 0);
    }
    return -1;
}
