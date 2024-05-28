#pragma once

namespace DLEngine
{
    class Model;

    class ModelManager
    {
    public:
        static Ref<Model> Load(const std::string& path);
        static Ref<Model> Get(const std::string& path);
        static bool Exists(const std::string& path);
    };
}
