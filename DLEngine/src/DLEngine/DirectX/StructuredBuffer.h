#pragma once
#include "DLEngine/DirectX/D3D.h"

#include "DLEngine/DirectX/View.h"

namespace DLEngine
{
    struct StructuredBufferDesc
    {
        size_t StructureSize{ 0u };
        uint32_t Count{ 1u };
        const void* Data{ nullptr };
    };

    class StructuredBuffer
    {
        friend class RenderCommand;
    public:
        void Create(const StructuredBufferDesc& desc);

        void Reset() noexcept { m_Handle.Reset(); m_StructureSize = 0u; m_Count = 0u; }

        void Resize(uint32_t count);

        void* Map() const;
        void Unmap() const;

        uint32_t GetElementsCount() const noexcept { return m_Count; }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_Handle{};
        size_t m_StructureSize{ 0u };
        uint32_t m_Count{ 0u };
    };

    class RStructuredBuffer
    {
    public:
        void Create(const StructuredBuffer& structuredBuffer);

        void Reset() noexcept { m_StructuredBuffer.Reset(); m_SRV.Reset(); }

        void Resize(uint32_t count);

        StructuredBuffer GetStructuredBuffer() const noexcept { return m_StructuredBuffer; }
        ShaderResourceView GetSRV() const noexcept { return m_SRV; }

    private:
        StructuredBuffer m_StructuredBuffer{};
        ShaderResourceView m_SRV{};
    };
}
