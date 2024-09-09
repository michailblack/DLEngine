#pragma once

namespace DLEngine
{
    enum class TextureFormat
    {
        None = 0,

        R8_UNORM,
        RG8_UNORM,
        RGBA8_UNORM,

        RG16_FLOAT,
        RGBA16_FLOAT,
        RGBA16_SNORM,

        RG32_FLOAT,
        RGBA32_FLOAT,

        R24_UNORM_X8_TYPELESS,

        BC1_UNORM,
        BC2_UNORM,
        BC3_UNORM,
        BC4_UNORM,
        BC4_SNORM,
        BC5_UNORM,
        BC5_SNORM,
        BC6H_UF16,
        BC6H_SF16,
        BC7_UNORM,
        BC7_UNORM_SRGB,

        DEPTH24STENCIL8,
        DEPTH_R24G8_TYPELESS,
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

    enum class TextureAddress
    {
        None = 0,
        Wrap,
        Clamp,
        Border,
    };

    enum class TextureFilter
    {
        None = 0,
        Nearest,
        Trilinear,
        Anisotropic8,
        BilinearCmp,
    };

    enum class PrimitiveTopology
    {
        None = 0,
        TrianglesList,
        TriangleStrip,
    };

    enum class FillMode
    {
        Solid = 0,
        Wireframe
    };

    enum class CullMode
    {
        None = 0,
        Front,
        Back
    };

    enum class CompareOperator
    {
        None = 0,
        Never,
        NotEqual,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        Equal,
        Always,
    };

    enum class StencilOperator
    {
        Keep = 0,
        Zero,
        Replace,
        IncrementAndClamp,
        DecrementAndClamp,
        Invert,
        IncrementAndWrap,
        DecrementAndWrap,
    };

    enum class BlendState
    {
        None = 0,

        AlphaToCoverage,
        General,
        PremultipliedAlpha,
        Additive,
    };

    enum ClearAttachment : uint8_t
    {
        DL_CLEAR_NONE               = BIT(0),
        DL_CLEAR_COLOR_ATTACHMENT   = BIT(1),
        DL_CLEAR_DEPTH_ATTACHMENT   = BIT(2),
        DL_CLEAR_STENCIL_ATTACHMENT = BIT(3)
    };
}