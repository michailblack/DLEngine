#pragma once
#include "DLEngine/DirectX/InputLayout.h"

#include "DLEngine/Systems/Mesh/Model.h"

#include "DLEngine/Systems/Transform/TransformSystem.h"

namespace DLEngine
{
    template <MaterialConcept TMaterial, typename TInstance>
    ShadingGroup<TMaterial, TInstance>::ShadingGroup(const ShadingGroupDesc& desc)
    {
        PipelineStateDesc pipelineStateSpec{};
        pipelineStateSpec = desc.PipelineDesc;

        InputLayout inputLayout;

        BufferLayout instanceBufferLayout{ desc.InstanceBufferLayout };
        instanceBufferLayout.AppendElement(
            BufferLayout::Element{ "TRANSFORM_INDEX", BufferLayout::ShaderDataType::Uint
        });
        inputLayout.AppendVertexBuffer(instanceBufferLayout, D3D11_INPUT_PER_INSTANCE_DATA);
        
        const auto vertexBufferLayout{ Model::GetCommonVertexBufferLayout() };
        inputLayout.AppendVertexBuffer(vertexBufferLayout, D3D11_INPUT_PER_VERTEX_DATA);
        inputLayout.Create(desc.PipelineDesc.VS);

        pipelineStateSpec.Layout = inputLayout;

        m_PipelineState.Create(pipelineStateSpec);

        m_InstanceBuffer.SetBufferLayout(instanceBufferLayout);
        m_MeshInstanceCB.Create();

        m_Render = desc.Render;
    }

    template <MaterialConcept TMaterial, typename TInstance>
    void ShadingGroup<TMaterial, TInstance>::AddModel(
        const Ref<Model>& model,
        std::vector<TMaterial> meshMaterials,
        const TInstance& instance,
        uint32_t transformID
    ) noexcept
    {
        DL_ASSERT(model->GetMeshesCount() == meshMaterials.size(), "meshMaterials must be the same size as the number of meshes in this model");

        auto modelInst{ std::ranges::find_if(m_Models, [&model](const PerModel& modelInst)
            {
                return modelInst.Model == model;
            })
        };

        if (modelInst == m_Models.end())
        {
            m_Models.emplace_back(PerModel{ model, {} });
            modelInst = m_Models.end() - 1;

            modelInst->Meshes.resize(model->GetMeshesCount());

            for (uint32_t meshIndex{ 0u }; meshIndex < model->GetMeshesCount(); ++meshIndex)
            {
                PerMaterial materialInst{};
                materialInst.Material = meshMaterials[meshIndex];
                materialInst.Instances.push_back({ instance, transformID });

                modelInst->Meshes[meshIndex].Materials.push_back(materialInst);
            }

            return;
        }

        for (uint32_t meshIndex{ 0u }; meshIndex < model->GetMeshesCount(); ++meshIndex)
        {
            auto materialInst{ std::ranges::find_if(modelInst->Meshes[meshIndex].Materials, [&meshMaterials, meshIndex](const PerMaterial& materialInst)
                {
                    return materialInst.Material == meshMaterials[meshIndex];
                })
            };

            if (materialInst == modelInst->Meshes[meshIndex].Materials.end())
            {
                PerMaterial newMaterialInst{};
                newMaterialInst.Material = meshMaterials[meshIndex];
                newMaterialInst.Instances.push_back({ instance, transformID });

                modelInst->Meshes[meshIndex].Materials.push_back(newMaterialInst);
                
                continue;
            }

            materialInst->Instances.push_back({ instance, transformID });
        }
    }

    template <MaterialConcept TMaterial, typename TInstance>
    void ShadingGroup<TMaterial, TInstance>::Render()
    {
        if (!m_Render)
            return;

        UpdateInstanceBuffer();

        m_PipelineState.Bind();
        m_InstanceBuffer.Bind(0u);
        m_MeshInstanceCB.Bind(4u, BIND_VS);

        uint32_t renderedInstances{ 0u };
        for (uint32_t modelIndex{ 0u }; modelIndex < m_Models.size(); ++modelIndex)
        {
            const auto& modelInst{ m_Models[modelIndex] };

            modelInst.Model->GetVertexBuffer().Bind(1u);
            modelInst.Model->GetIndexBuffer().Bind();

            for (uint32_t meshIndex{ 0u }; meshIndex < modelInst.Meshes.size(); ++meshIndex)
            {
                const auto& mesh{ modelInst.Model->GetMesh(meshIndex) };
                const auto& meshRange{ modelInst.Model->GetMeshRange(meshIndex) };

                for (const auto& materialInst : modelInst.Meshes[meshIndex].Materials)
                {
                    materialInst.Material.Set();

                    uint32_t instanceCount{ static_cast<uint32_t>(materialInst.Instances.size()) };

                    // TODO: Think about handling multiple instances of one mesh in a model
                    for (uint32_t meshInstanceIndex{ 0u }; meshInstanceIndex < mesh.GetInstanceCount(); ++meshInstanceIndex)
                    {
                        MeshInstance meshInstance{};
                        meshInstance.MeshToModel = mesh.GetInstance(meshInstanceIndex);
                        meshInstance.ModelToMesh = mesh.GetInvInstance(meshInstanceIndex);

                        m_MeshInstanceCB.Set(&meshInstance, 1u);

                        DL_THROW_IF_D3D11(D3D::GetDeviceContext4()->DrawIndexedInstanced(
                            meshRange.IndexCount,
                            instanceCount,
                            meshRange.IndexOffset,
                            meshRange.VertexOffset,
                            renderedInstances
                        ));
                    }
                    renderedInstances += instanceCount;
                }
            }
        }
    }


    template <MaterialConcept TMaterial, typename TInstance>
    bool ShadingGroup<TMaterial, TInstance>::Intersects(
        const Math::Ray& ray,
        IShadingGroup::IntersectInfo& outIntersectInfo
    ) const noexcept
    {
        bool intersects{ false };
        for (uint32_t modelIndex{ 0u }; modelIndex < m_Models.size(); ++modelIndex)
        {
            const auto& modelInst{ m_Models[modelIndex] };

            for (uint32_t meshIndex{ 0u }; meshIndex < modelInst.Meshes.size(); ++meshIndex)
            {
                const auto& meshInst{ modelInst.Meshes[meshIndex] };
                const auto& mesh{ modelInst.Model->GetMesh(meshIndex) };

                for (const auto& materialInst : meshInst.Materials)
                {
                    for (const auto& meshInstance : materialInst.Instances)
                    {
                        const auto& modelToWorld{ TransformSystem::GetTransform(meshInstance.TransformID) };
                        const auto& worldToModel{ TransformSystem::GetInvTransform(meshInstance.TransformID) };

                        const Math::Ray transformedRay{
                            TransformSystem::TransformPoint(ray.Origin, worldToModel),
                            Math::Normalize(TransformSystem::TransformDirection(ray.Direction, worldToModel))
                        };

                        Mesh::IntersectInfo intersectInfo{ outIntersectInfo.MeshIntersectInfo };
                        if (intersectInfo.TriangleIntersectInfo.T != Math::Numeric::Inf)
                        {
                            intersectInfo.TriangleIntersectInfo.IntersectionPoint = TransformSystem::TransformPoint(intersectInfo.TriangleIntersectInfo.IntersectionPoint, worldToModel);
                            intersectInfo.TriangleIntersectInfo.Normal = Math::Normalize(TransformSystem::TransformDirection(intersectInfo.TriangleIntersectInfo.Normal, worldToModel));
                            intersectInfo.TriangleIntersectInfo.T = Math::Length(intersectInfo.TriangleIntersectInfo.IntersectionPoint - transformedRay.Origin);
                        }

                        if (mesh.Intersects(transformedRay, intersectInfo))
                        {
                            outIntersectInfo.Model = modelInst.Model;
                            outIntersectInfo.TransformID = meshInstance.TransformID;

                            auto& triangleIntersectInfo{ outIntersectInfo.MeshIntersectInfo.TriangleIntersectInfo };
                            triangleIntersectInfo.IntersectionPoint = TransformSystem::TransformPoint(
                                intersectInfo.TriangleIntersectInfo.IntersectionPoint, modelToWorld
                            );
                            triangleIntersectInfo.Normal = Math::Normalize(
                                TransformSystem::TransformDirection(intersectInfo.TriangleIntersectInfo.Normal, modelToWorld)
                            );
                            triangleIntersectInfo.T = Math::Length(triangleIntersectInfo.IntersectionPoint - ray.Origin);

                            outIntersectInfo.MeshIntersectInfo.TriangleIndex = intersectInfo.TriangleIndex;
                            outIntersectInfo.MeshIntersectInfo.InstanceIndex = intersectInfo.InstanceIndex;

                            intersects = true;
                        }
                    }
                }
            }
        }
        return intersects;
    }

    template <MaterialConcept TMaterial, typename TInstance>
    void ShadingGroup<TMaterial, TInstance>::UpdateInstanceBuffer() noexcept
    {
        uint32_t totalInstances{ 0u };
        for (const auto& modelInst : m_Models)
            for (const auto& meshInst : modelInst.Meshes)
                for (const auto& materialInst : meshInst.Materials)
                    totalInstances += static_cast<uint32_t>(materialInst.Instances.size());

        if (totalInstances == 0)
            return;

        m_InstanceBuffer.Resize(totalInstances);

        auto* instanceBufferPtr{ m_InstanceBuffer.Map() };

        uint32_t copiedCount{ 0u };
        for (const auto& modelInst : m_Models)
        {
            for (const auto& meshInst : modelInst.Meshes)
            {
                for (const auto& materialInst : meshInst.Materials)
                {
                    for (const auto& instance : materialInst.Instances)
                    {
                        instanceBufferPtr[copiedCount++] = {
                            .Instance = instance.Instance,
                            .TransformIndex = TransformSystem::GetArrayIndex(instance.TransformID)
                        };
                    }
                }
            }
        }

        m_InstanceBuffer.Unmap();
    }    
}
