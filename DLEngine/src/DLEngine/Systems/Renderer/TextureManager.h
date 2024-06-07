#pragma once
#include "DLEngine/DirectX/Texture.h"
#include "DLEngine/DirectX/View.h"

namespace DLEngine
{
    struct Texture2DEntry
    {
        Texture2D Texture{};
        ShaderResourceView SRV{};
    };

    class TextureManager
    {
    public:
        static void Init();

        static Texture2DEntry LoadTexture2D(const std::wstring& path);
        static Texture2DEntry GetTexture2D(const std::wstring& path);
        static bool Exists2D(const std::wstring& path);
    };
}
