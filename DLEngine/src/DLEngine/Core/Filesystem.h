#pragma once
#include <string>

namespace DLEngine
{
    class Filesystem
    {
    public:
        static void Init();

        static std::string GetShaderDir() noexcept;
        static std::string GetModelDir() noexcept;
    };
}

