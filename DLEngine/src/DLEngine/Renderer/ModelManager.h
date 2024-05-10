#pragma once

namespace DLEngine
{
    class Model;

    // TODO: Set working directory at the start of the program,
    // so every path converts to its absolute representation, which makes it unique
    class ModelManager
    {
    public:
        static Ref<Model> Load(const std::string& path);
        static Ref<Model> Get(const std::string& path);
        static bool Exists(const std::string& path);
    };
}
