#pragma once
#include <string>

namespace DLEngine
{
    class Filesystem
    {
    public:
        static void Init();

        static std::wstring GetShaderDir() noexcept;
        static std::wstring GetModelDir() noexcept;
    };
}

