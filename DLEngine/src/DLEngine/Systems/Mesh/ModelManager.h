#pragma once

namespace DLEngine
{
    class Model;

    class ModelManager
    {
    public:
        static void Init();

        static Ref<Model> Load(const std::wstring& path);
        static Ref<Model> Get(const std::wstring& path);
        static bool Exists(const std::wstring& path);

    private:
        static void InitUnitSphere();
    };
}
