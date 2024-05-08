#pragma once
#include "DLEngine/Core/Engine.h"
#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/DLException.h"

int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    try
    {
        DLEngine::Engine::Init();

        const Scope<DLEngine::Application> app { DLEngine::CreateApplication() };
        app->Run();

        DLEngine::Engine::Deinit();

        return 0;
    }
    catch (const DLEngine::DLException& e)
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

    DLEngine::Engine::Deinit();

    return -1;
}
