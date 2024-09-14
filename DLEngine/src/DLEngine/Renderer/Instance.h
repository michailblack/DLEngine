#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Buffer.h"

#include "DLEngine/Renderer/Shader.h"

#include <string>

namespace DLEngine
{
    class Instance
    {
    public:
        virtual ~Instance() = default;

        virtual void Set(const std::string& name, const Buffer& buffer) noexcept = 0;

        virtual const Buffer Get(const std::string& name) const noexcept = 0;

        template <typename T>
        const T& Get(const std::string& name) const noexcept
        {
            const auto& buffer{ Get(name) };

            DL_ASSERT(buffer.Size == sizeof(T), "Element size mismatch for element [{0}] in the instance [{1}]", name, GetName());

            return buffer.Read<T>();
        }

        virtual bool HasUniform(const std::string& name) const noexcept = 0;

        virtual const Buffer& GetInstanceData() const noexcept = 0;
        virtual Ref<Shader> GetShader() const noexcept = 0;
        virtual const std::string& GetName() const noexcept = 0;

        static Ref<Instance> Create(const Ref<Shader>& shader, const std::string& name = "");
        static Ref<Instance> Copy(const Ref<Instance>& instance, const std::string& name = "");
        static Ref<Instance> Copy(const Ref<Instance>& instance, const Ref<Shader>& differentShader, const std::string& name = "");
    };
}