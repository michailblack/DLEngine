#pragma once
#include "DLEngine/Renderer/Instance.h"

namespace DLEngine
{
    class D3D11Instance : public Instance
    {
    public:
        D3D11Instance(const Ref<Shader>& shader, const std::string& name) noexcept;
        D3D11Instance(const Ref<Instance>& instance, const std::string& name) noexcept;
        ~D3D11Instance() override;

        void Set(const std::string& name, const Buffer& buffer) noexcept override;

        const Buffer Get(const std::string& name) const noexcept override;

        bool HasUniform(const std::string& name) const noexcept override { return m_ElementMap.contains(name); }

        const Buffer& GetInstanceData() const noexcept override { return m_InstanceData; }
        Ref<Shader> GetShader() const noexcept override { return m_Shader; }
        const std::string& GetName() const noexcept override { return m_Name; }

    private:
        void GenerateElementMap(const VertexBufferLayout& instanceLayout, size_t offset) noexcept;

    private:
        std::string m_Name;

        Ref<Shader> m_Shader;

        Buffer m_InstanceData;

        std::unordered_map<std::string, std::pair<const VertexBufferElement*, size_t>> m_ElementMap;
    };
}