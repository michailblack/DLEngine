#pragma once
#include "DLEngine/DirectX/D3D.h"

namespace DLEngine
{
    class StructuredBuffer
    {
    public:
        void Create(size_t structureSize, uint32_t count = 1u, const void* data = nullptr);

        void Reset() noexcept { m_Handle.Reset(); m_SRV.Reset(); m_StructureSize = 0u; m_Count = 0u; }

        void Resize(uint32_t count);

        void* Map() const;
        void Unmap() const noexcept;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> GetSRV() const noexcept { return m_SRV; }

        uint32_t GetElementsCount() const noexcept { return m_Count; }

    private:
        void CreateSRV();

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_Handle{};
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView1> m_SRV{};
        size_t m_StructureSize{ 0u };
        uint32_t m_Count{ 0u };
    };
}
