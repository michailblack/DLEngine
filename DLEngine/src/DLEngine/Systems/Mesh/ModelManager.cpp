#include "dlpch.h"
#include "ModelManager.h"

#include "DLEngine/Systems/Mesh/Model.h"

namespace DLEngine
{
    namespace
    {
        struct ModelManagerData
        {
            std::unordered_map<std::wstring, Ref<Model>> Models;
        } s_Data;
    }

    Ref<Model> ModelManager::Load(const std::wstring& path)
    {
        if (Exists(path))
            return s_Data.Models[path];

        const auto& [it, hasConstructed]{ s_Data.Models.emplace(std::make_pair(path, CreateRef<Model>(path))) };

        return it->second;
    }

    Ref<Model> ModelManager::Get(const std::wstring& path)
    {
        DL_ASSERT_NOINFO(Exists(path));

        return s_Data.Models[path];
    }

    bool ModelManager::Exists(const std::wstring& path)
    {
        return s_Data.Models.contains(path);
    }
}
