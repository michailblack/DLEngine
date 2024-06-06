#pragma once
#include "DLEngine/DirectX/Texture2D.h"

namespace DLEngine
{
    class TextureManager
    {
    public:
        static void Init();

        static Texture2D Load(const std::wstring& path);
        static Texture2D Get(const std::wstring& path);
        static bool Exists(const std::wstring& path);

    private:
        static Texture2D LoadFromFile(const std::wstring& path);
    };
}
