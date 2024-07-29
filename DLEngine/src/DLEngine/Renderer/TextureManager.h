#pragma once
#include "DLEngine/DirectX/Texture.h"

#include "DLEngine/Math/Vec4.h"

namespace DLEngine
{
    class TextureManager
    {
    public:
        static void Init();

        static Texture2D LoadTexture2D(const std::wstring& path);
        static Texture2D GetTexture2D(const std::wstring& path);

        static Texture2D GenerateValueTexture2D(Math::Vec4 value);

        static void SaveToDDS(const Texture2D& texture, const std::wstring& name);

        static bool Exists2D(const std::wstring& path);
    };
}
