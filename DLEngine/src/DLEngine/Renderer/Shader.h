#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Buffer.h"

#include "DLEngine/Renderer/ShaderInput.h"

#include <array>
#include <filesystem>

namespace DLEngine
{
    enum ShaderStage : uint8_t
    {
        DL_VERTEX_SHADER_BIT     = BIT(1),
        DL_HULL_SHADER_BIT       = BIT(2),
        DL_DOMAIN_SHADER_BIT     = BIT(3),
        DL_GEOMETRY_SHADER_BIT   = BIT(4),
        DL_PIXEL_SHADER_BIT      = BIT(5),
        DL_COMPUTE_SHADER_BIT    = BIT(6),

        DL_ALL_SHADER_STAGES = DL_VERTEX_SHADER_BIT |
        DL_HULL_SHADER_BIT |
        DL_DOMAIN_SHADER_BIT |
        DL_GEOMETRY_SHADER_BIT |
        DL_PIXEL_SHADER_BIT |
        DL_COMPUTE_SHADER_BIT
    };

    class ShaderUniform
    {
    public:
        ShaderUniform(const std::string& name, ShaderDataType type, uint32_t size, uint32_t offset)
            : m_Name(name), m_Type(type), m_Size(size), m_Offset(offset)
        {}

        const std::string& GetName() const noexcept { return m_Name; }
        ShaderDataType GetType() const noexcept { return m_Type; }
        uint32_t GetSize() const noexcept { return m_Size; }
        uint32_t GetOffset() const noexcept { return m_Offset; }

    private:
        std::string m_Name{};
        ShaderDataType m_Type{ ShaderDataType::None };
        uint32_t m_Size{ 0u };
        uint32_t m_Offset{ 0u };
    };

    struct ShaderBuffer
    {
        std::unordered_map<std::string, ShaderUniform> Uniforms{};
        std::string Name{};
        uint32_t Size{ 0u };
        uint32_t BindPoint{ 0u };
        uint8_t ShaderStageFlags{ 0u };
    };

    enum class ShaderTextureType
    {
        None = 0,
        Texture2D, Texture2DArray,
        TextureCube, TextureCubeArray
    };

    struct ShaderTexture
    {
        std::string Name{};
        ShaderTextureType Type{ ShaderTextureType::None };
        uint32_t BindPoint{ 0u };
        uint8_t ShaderStageFlags{ 0u };
    };

    struct ShaderDefine
    {
        std::string Name{};
        std::string Value{};
    };

    enum class InputLayoutType
    {
        PerVertex,
        PerInstance
    };

    struct InputLayoutSpecification
    {
        VertexBufferLayout Layout{};
        InputLayoutType Type{ InputLayoutType::PerVertex };
        uint32_t InstanceStepRate{ 0u };

        InputLayoutSpecification() = default;

        InputLayoutSpecification(const VertexBufferLayout& layout, InputLayoutType type, uint32_t instanceStepRate)
            : Layout(layout), Type(type), InstanceStepRate(instanceStepRate)
        {}
    };

    struct ShaderSpecification
    {
        std::unordered_map<ShaderStage, std::string_view> EntryPoints{};
        std::filesystem::path Path{};
        std::map<uint32_t, InputLayoutSpecification> InputLayouts{};
        std::vector<ShaderDefine> Defines{};
    };

    class Shader
    {
    public:
        virtual ~Shader() = default;

        virtual const std::string& GetName() const noexcept = 0;

        virtual const std::map<uint32_t, InputLayoutSpecification>& GetInputLayout() const noexcept = 0;

        static Ref<Shader> Create(const ShaderSpecification& specification);

        static const std::filesystem::path GetShaderDirectoryPath() noexcept;
    };

    class ShaderLibrary
    {
    public:
        void Init();

        void Add(const Ref<Shader>& shader) noexcept;
        Ref<Shader> Load(const ShaderSpecification& specification) noexcept;
        Ref<Shader> Get(const std::string_view name) noexcept;

    private:
        std::unordered_map<std::string_view, Ref<Shader>> m_Shaders{};
    };
}