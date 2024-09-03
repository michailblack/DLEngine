#pragma once
#include "DLEngine/Core/Base.h"
#include "DLEngine/Core/Buffer.h"

#include "DLEngine/Renderer/Sampler.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace DLEngine
{
    namespace Math
    {
        class Vec2;
    }

    enum class TextureFormat
    {
        None = 0,
        RG16F,
        RG32F,
        RGBA,
        RGBA16F,
        RGBA32F,
        R24X8,

        BC1U,
        BC2U,
        BC3U,
        BC4U,
        BC4S,
        BC5U,
        BC5S,
        BC6H,
        BC6S,
        BC7U,

        DEPTH24STENCIL8,
        DEPTH_R24G8T,
    };

    enum class TextureUsage
    {
        None = 0,
        Texture,           // Can be sampled from in shader
        Attachment,        // Can be written to in shader
        TextureAttachment, // Can be sampled from and written to in shader
    };

    enum class TextureType
    {
        None = 0,
        Texture2D,
        TextureCube,
    };

    struct TextureSpecification
    {
        std::string DebugName;

        TextureFormat Format;
        TextureUsage Usage;

        uint32_t Width{ 1u };
        uint32_t Height{ 1u };
        
        uint32_t Mips{ 1u };
        uint32_t Layers{ 1u };

        uint32_t Samples{ 1u };
    };

    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual TextureType GetType() const noexcept = 0;

        virtual uint32_t GetWidth() const noexcept = 0;
        virtual uint32_t GetHeight() const noexcept = 0;
        virtual Math::Vec2 GetSize() const noexcept = 0;

        virtual uint32_t GetMipsCount() const noexcept = 0;
        virtual uint32_t GetLayersCount() const noexcept = 0;
        virtual uint32_t GetSamplesCount() const noexcept = 0;

        virtual const TextureSpecification& GetSpecification() const noexcept = 0;

        virtual const std::filesystem::path& GetPath() const noexcept = 0;

        static const std::filesystem::path GetTextureDirectoryPath() noexcept;
    };

    class Texture2D : public Texture
    {
    public:
        static TextureType GetStaticType() noexcept { return TextureType::Texture2D; }
        TextureType GetType() const noexcept override { return GetStaticType(); }

        static Ref<Texture2D> Create(const TextureSpecification& specification);
        static Ref<Texture2D> Create(const TextureSpecification& specification, const std::filesystem::path& path);
    };

    class TextureCube : public Texture
    {
    public:
        static TextureType GetStaticType() noexcept { return TextureType::TextureCube; }
        TextureType GetType() const noexcept override { return GetStaticType(); }

        static Ref<TextureCube> Create(const TextureSpecification& specification);
        static Ref<TextureCube> Create(const TextureSpecification& specification, const std::filesystem::path& path);
    };

    class TextureLibrary
    {
    public:
        void Add(const Ref<Texture>& texture);
        Ref<Texture2D> LoadTexture2D(const TextureSpecification& specification, const std::filesystem::path& path);
        Ref<TextureCube> LoadTextureCube(const TextureSpecification& specification, const std::filesystem::path& path);
        Ref<Texture> Get(const std::filesystem::path& path) const;

    private:
        std::unordered_map<std::filesystem::path, Ref<Texture>> m_Textures;
    };

    struct TextureSubresource
    {
        uint32_t BaseMip{ 0u };
        uint32_t MipsCount{ static_cast<uint32_t>(-1) };
        uint32_t BaseLayer{ 0u };
        uint32_t LayersCount{ 1u };

        bool operator==(const TextureSubresource& other) const noexcept
        {
            return BaseMip == other.BaseMip &&
                MipsCount == other.MipsCount &&
                BaseLayer == other.BaseLayer &&
                LayersCount == other.LayersCount;
        }
    };

    struct TextureViewSpecification
    {
        TextureSubresource Subresource{};
        TextureFormat Format{ TextureFormat::None };

        bool operator==(const TextureViewSpecification& other) const noexcept
        {
            return Format == other.Format && Subresource == other.Subresource;
        }
    };

    struct TextureViewSpecificationHash
    {
        std::size_t operator()(const TextureViewSpecification& specification) const noexcept
        {
            return std::hash<TextureFormat>{}(specification.Format) ^
                std::hash<uint32_t>{}(specification.Subresource.BaseMip) ^
                std::hash<uint32_t>{}(specification.Subresource.MipsCount) ^
                std::hash<uint32_t>{}(specification.Subresource.BaseLayer) ^
                std::hash<uint32_t>{}(specification.Subresource.LayersCount);
        }
    };
    
    namespace Utils
    {
        inline bool IsDepthFormat(TextureFormat format)
        {
            return format == TextureFormat::DEPTH24STENCIL8 ||
                format == TextureFormat::DEPTH_R24G8T;
        }
    }
}