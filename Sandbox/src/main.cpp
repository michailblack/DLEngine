#include "Application.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    auto* app = new Application({ 800, 600, L"Simple Ray Casting" });
    app->Init();
    app->Run();
    delete app;
}