#include "dlpch.h"
#include "ModelManager.h"

#include "DLEngine/Renderer/Model.h"

namespace DLEngine
{
    namespace
    {
        struct ModelManagerData
        {
            std::unordered_map<std::string, Ref<Model>> Models;
        } s_Data;
    }

    Ref<Model> ModelManager::Load(const std::string& path)
    {
        if (Exists(path))
            return s_Data.Models[path];

        const auto& [it, hasConstructed]{ s_Data.Models.emplace(std::make_pair(path, CreateRef<Model>(path))) };

        return it->second;
    }

    Ref<Model> ModelManager::Get(const std::string& path)
    {
        DL_ASSERT_NOINFO(Exists(path));

        return s_Data.Models[path];
    }

    bool ModelManager::Exists(const std::string& path)
    {
        return s_Data.Models.contains(path);
    }
}
