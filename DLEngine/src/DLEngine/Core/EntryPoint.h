#pragma once
#include "DLEngine/Core/Application.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    DL_ASSERT(DirectX::XMVerifyCPUSupport(), "DirectXMath Library does not support the given platform")

    auto* app = CreateApplication();
    app->Run();
    delete app;
}