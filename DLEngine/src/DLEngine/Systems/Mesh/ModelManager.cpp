#include "dlpch.h"
#include "ModelManager.h"

#include "DLEngine/Systems/Mesh/Model.h"

namespace DLEngine
{
    namespace
    {
        struct
        {
            std::unordered_map<std::string, Ref<Model>> Models;
        } s_ModelManagerData;
    }

    Ref<Model> ModelManager::Load(const std::string& path)
    {
        if (Exists(path))
            return s_ModelManagerData.Models[path];

        const auto& [it, hasConstructed]{ s_ModelManagerData.Models.emplace(std::make_pair(path, CreateRef<Model>(path))) };

        return it->second;
    }

    Ref<Model> ModelManager::Get(const std::string& path)
    {
        DL_ASSERT_NOINFO(Exists(path));

        return s_ModelManagerData.Models[path];
    }

    bool ModelManager::Exists(const std::string& path)
    {
        return s_ModelManagerData.Models.contains(path);
    }
}
