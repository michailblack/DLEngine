#include "dlpch.h"
#include "Filesystem.h"

#include "DLEngine/Core/Application.h"

#include "DLEngine/DirectX/Shaders.h"

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

        DL_LOG_INFO(L"Shader directory: {0}", GetShaderDir());
        DL_LOG_INFO(L"Model directory: {0}", GetModelDir());
    }

    std::wstring Filesystem::GetShaderDir() noexcept
    {
        return s_FilesystemData.ShaderDir.wstring();
    }

    std::wstring Filesystem::GetModelDir() noexcept
    {
        return s_FilesystemData.ModelDir.wstring();
    }
}
