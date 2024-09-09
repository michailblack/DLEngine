#include "dlpch.h"
#include "D3D11Shader.h"

#include "DLEngine/DirectX/D3D11ShaderCompiler.h"

namespace DLEngine
{
    D3D11Shader::D3D11Shader(const ShaderSpecification& specification)
        : m_Specification(specification), m_Name(specification.Path.stem().string())
    {
        D3D11ShaderCompiler compiler{ this };
        compiler.Compile();

        DL_LOG_INFO_TAG("Shader", "Shader [{0}] compiled successfully", m_Name);
    }
}