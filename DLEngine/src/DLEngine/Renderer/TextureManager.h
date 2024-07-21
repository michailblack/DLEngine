#pragma once
#include "DLEngine/DirectX/Texture.h"

#include "DLEngine/Math/Vec4.h"

namespace DLEngine
{
    class TextureManager
    {
    public:
        static void Init();

        static RTexture2D LoadTexture2D(const std::wstring& path);
        static RTexture2D GetTexture2D(const std::wstring& path);

        static RTexture2D GenerateValueTexture2D(Math::Vec4 value);

        static bool Exists2D(const std::wstring& path);
    };
}
