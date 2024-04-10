#pragma once
#include "DLEngine/Core/Application.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    auto* app = CreateApplication();
    app->Run();
    delete app;
}