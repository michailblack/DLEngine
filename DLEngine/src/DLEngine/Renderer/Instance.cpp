#include "dlpch.h"
#include "Instance.h"

#include "DLEngine/DirectX/D3D11Instance.h"

namespace DLEngine
{
    Ref<Instance> Instance::Create(const Ref<Shader>& shader, const std::string& name)
    {
        return CreateRef<D3D11Instance>(shader, name);
    }

    Ref<Instance> Instance::Copy(const Ref<Instance>& instance, const std::string& name)
    {
        return CreateRef<D3D11Instance>(instance, name);
    }

}