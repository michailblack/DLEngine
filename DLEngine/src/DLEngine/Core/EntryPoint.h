#pragma once
#include "DLEngine/Core/Application.h"
#include "DLEngine/Core/DLException.h"

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
{
    try
    {
        const DLEngine::Scope<DLEngine::Application> app {
            DLEngine::CreateApplication(std::wstring{ lpCmdLine, lpCmdLine + strlen(lpCmdLine) })
        };
        app->Run();

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

    return -1;
}
