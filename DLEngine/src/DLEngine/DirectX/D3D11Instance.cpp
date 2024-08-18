#include "dlpch.h"
#include "D3D11Instance.h"

namespace DLEngine
{
    D3D11Instance::D3D11Instance(const Ref<Shader>& shader, const std::string& name) noexcept
        : m_Name(name), m_Shader(shader)
    {
        const auto& instanceLayout{ shader->GetInstanceLayout() };
        GenerateElementMap(instanceLayout);

        const auto instanceDataSize{ instanceLayout.GetStride() };
        m_InstanceData.Allocate(instanceDataSize);
    }

    D3D11Instance::D3D11Instance(const Ref<Instance>& instance, const std::string& name) noexcept
        : m_Name(name), m_Shader(instance->GetShader())
    {
        const auto& d3d11Instance{ AsRef<D3D11Instance>(instance) };

        m_InstanceData = Buffer::Copy(d3d11Instance->m_InstanceData);
        m_ElementMap = d3d11Instance->m_ElementMap;
    }

    D3D11Instance::~D3D11Instance()
    {
        m_InstanceData.Release();
    }

    void D3D11Instance::Set(const std::string& name, const Buffer& buffer) noexcept
    {
        const auto& it{ m_ElementMap.find(name) };

        DL_ASSERT(it != m_ElementMap.end(), "Failed to find element with name [{0}] in the instance [{1}]", name, m_Name);

        const auto& element{ it->second };
        m_InstanceData.Write(buffer.Data, buffer.Size, element->Offset);
    }

    const Buffer D3D11Instance::Get(const std::string& name) const noexcept
    {
        const auto& it{ m_ElementMap.find(name) };

        DL_ASSERT(it != m_ElementMap.end(), "Failed to find element with name [{0}] in the instance [{1}]", name, m_Name);

        const auto& element{ it->second };
        return Buffer{ m_InstanceData.ReadRaw(element->Offset), element->Size };
    }

    void D3D11Instance::GenerateElementMap(const VertexBufferLayout& instanceLayout) noexcept
    {
        for (const auto& element : instanceLayout)
            m_ElementMap.emplace(std::make_pair(element.Name, &element));
    }

}