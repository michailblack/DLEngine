#include "dlpch.h"
#include "Material.h"

#include "DLEngine/DirectX/D3D11Material.h"

namespace DLEngine
{
    Ref<Material> Material::Create(const Ref<Shader>& shader, const std::string& name)
    {
        return CreateRef<D3D11Material>(shader, name);
    }

    Ref<Material> Material::Copy(const Ref<Material>& material, const std::string& name)
    {
        return CreateRef<D3D11Material>(material, name);
    }

}