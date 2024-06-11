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
            std::filesystem::path TextureDir;
        } s_FilesystemData;
    }

    void Filesystem::Init()
    {
        std::filesystem::path workingDir{ Application::Get().GetWorkingDir() };
        s_FilesystemData.ShaderDir = workingDir / "DLEngine" / "src" / "DLEngine" / "Shaders\\";
        s_FilesystemData.ModelDir = workingDir / "assets" / "models\\";
        s_FilesystemData.TextureDir = workingDir / "assets" / "textures\\";

        DL_LOG_INFO("Shader directory: {}", Utils::WideStrToMultiByteStr(GetShaderDir()));
        DL_LOG_INFO("Model directory: {}", Utils::WideStrToMultiByteStr(GetModelDir()));
        DL_LOG_INFO("Texture directory: {}", Utils::WideStrToMultiByteStr(GetTextureDir()));
    }

    std::wstring Filesystem::GetShaderDir() noexcept
    {
        return s_FilesystemData.ShaderDir.wstring();
    }

    std::wstring Filesystem::GetModelDir() noexcept
    {
        return s_FilesystemData.ModelDir.wstring();
    }

    std::wstring Filesystem::GetTextureDir() noexcept
    {
        return s_FilesystemData.TextureDir.wstring();
    }
}
