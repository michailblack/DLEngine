#pragma once
#include "DLEngine/DirectX/ConstantBuffers.h"
#include "DLEngine/DirectX/InputLayout.h"
#include "DLEngine/DirectX/Shaders.h"

#include "DLEngine/Math/Mat4x4.h"

#include "DLEngine/Renderer/Model.h"

namespace DLEngine
{
    class HologramGroup
    {
    public:
        struct Instance
        {
            Math::Mat4x4 ModelToWorld;
            Math::Vec3 BaseColor;
            Math::Vec3 AdditionalColor;
        };

        struct PerMesh
        {
            std::vector<Instance> Instances;
        };

        struct PerModel
        {
            Ref<Model> Model;
            std::vector<PerMesh> PerMesh;
        };

    public:
        HologramGroup();
        ~HologramGroup() = default;

        HologramGroup(const HologramGroup&) = delete;
        HologramGroup(HologramGroup&&) = delete;
        HologramGroup& operator=(const HologramGroup&) = delete;
        HologramGroup& operator=(HologramGroup&&) = delete;

        void AddModel(const Ref<Model>& model, const std::vector<Instance>& instances);
        void Render();

        const std::vector<PerModel>& GetModels() const noexcept { return m_Models; }

    private:
        void UpdateInstanceBuffer() const;

    private:
        struct PerDraw
        {
            Math::Mat4x4 MeshToModel;
            Math::Mat4x4 ModelToMesh;
        };

    private:
        std::vector<PerModel> m_Models;
        Ref<PerInstanceBuffer<Instance>> m_InstanceBuffer;

        Ref<ConstantBuffer<PerDraw>> m_PerDrawConstantBuffer;

        Ref<InputLayout> m_InputLayout;

        Ref<VertexShader> m_VertexShader;
        Ref<PixelShader> m_PixelShader;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState2> m_RasterizerState;
    };
}
