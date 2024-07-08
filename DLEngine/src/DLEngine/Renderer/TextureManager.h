#pragma once
#include "DLEngine/DirectX/Texture.h"
#include "DLEngine/DirectX/View.h"

namespace DLEngine
{
    struct Texture2DResource
    {
        Texture2D Texture{};
        ShaderResourceView SRV{};
    };

    class TextureManager
    {
    public:
        static void Init();

        static Texture2DResource LoadTexture2D(const std::wstring& path);
        static Texture2DResource GetTexture2D(const std::wstring& path);

        static Texture2DResource GenerateValueTexture2D(Math::Vec4 value);

        static bool Exists2D(const std::wstring& path);
    };
}
