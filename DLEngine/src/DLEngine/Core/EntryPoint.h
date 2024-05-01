#pragma once
#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/DLException.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    try
    {
        if (!DirectX::XMVerifyCPUSupport())
            throw std::runtime_error {"DirectXMath Library does not support the given platform" };

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
