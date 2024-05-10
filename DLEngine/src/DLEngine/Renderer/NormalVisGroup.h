#pragma once
#include "DLEngine/DirectX/ConstantBuffers.h"
#include "DLEngine/DirectX/InputLayout.h"
#include "DLEngine/DirectX/Shaders.h"

#include "DLEngine/Math/Mat4x4.h"
#include "DLEngine/Math/Primitives.h"

#include "DLEngine/Renderer/Model.h"

namespace DLEngine
{
    class NormalVisGroup
    {
    public:
        struct Instance
        {
            Math::Mat4x4 ModelToWorld;
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
        NormalVisGroup();
        ~NormalVisGroup() = default;

        NormalVisGroup(const NormalVisGroup&) = delete;
        NormalVisGroup(NormalVisGroup&&) = delete;
        NormalVisGroup& operator=(const NormalVisGroup&) = delete;
        NormalVisGroup& operator=(NormalVisGroup&&) = delete;

        void AddModel(const Ref<Model>& model, const std::vector<Instance>& instances);
        void Render();

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
