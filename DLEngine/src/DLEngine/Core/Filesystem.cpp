#include "dlpch.h"
#include "Filesystem.h"

#include "DLEngine/Core/Application.h"

#include <filesystem>

namespace DLEngine
{
    namespace
    {
        struct
        {
            std::filesystem::path ShaderDir;
            std::filesystem::path ModelDir;
        } s_FilesystemData;
    }

    void Filesystem::Init()
    {
        std::filesystem::path workingDir{ Application::Get().GetWorkingDir() };
        s_FilesystemData.ShaderDir = workingDir / "DLEngine" / "src" / "DLEngine" / "Shaders\\";
        s_FilesystemData.ModelDir = workingDir / "models\\";

        DL_LOG_INFO("Shader directory: {0}", GetShaderDir());
        DL_LOG_INFO("Model directory: {0}", GetModelDir());
    }

    std::string Filesystem::GetShaderDir() noexcept
    {
        return s_FilesystemData.ShaderDir.string();
    }

    std::string Filesystem::GetModelDir() noexcept
    {
        return s_FilesystemData.ModelDir.string();
    }
}
