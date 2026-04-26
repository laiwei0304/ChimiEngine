#pragma once

#include "ECS/System/CmMaterialSystem.h"
#include "ECS/Component/Material/CmUnlitMaterialComponent.h"

namespace chimi{
#define NUM_MATERIAL_BATCH              16
#define NUM_MATERIAL_BATCH_MAX          2048

    class CmVKPipelineLayout;
    class CmVKPipeline;
    class CmVKDescriptorSetLayout;
    class CmVKDescriptorPool;

    class CmUnlitMaterialSystem : public CmMaterialSystem{
    public:
        void OnInit(CmVKRenderPass *renderPass) override;
        void OnRender(VkCommandBuffer cmdBuffer, CmRenderTarget *renderTarget, uint32_t frameIndex) override;
        void OnDestroy() override;
    private:
        void InitFrameUboDescriptors();
        void EnsureMaterialCapacity(uint32_t frameIndex, uint32_t materialCount);
        void UpdateFrameUbo(uint32_t frameIndex, CmRenderTarget *renderTarget);
        void UpdateMaterialParamsDescSet(uint32_t frameIndex, uint32_t materialIndex, CmUnlitMaterial *material);
        bool UpdateMaterialResourceDescSet(uint32_t frameIndex, uint32_t materialIndex, CmUnlitMaterial *material);
        bool AreMaterialParamsSynced(uint32_t materialIndex, const CmUnlitMaterial *material) const;
        bool AreMaterialResourcesSynced(uint32_t materialIndex, const CmUnlitMaterial *material) const;

        std::shared_ptr<CmVKDescriptorSetLayout> mFrameUboDescSetLayout;
        std::shared_ptr<CmVKDescriptorSetLayout> mMaterialParamDescSetLayout;
        std::shared_ptr<CmVKDescriptorSetLayout> mMaterialResourceDescSetLayout;

        std::shared_ptr<CmVKPipelineLayout> mPipelineLayout;
        std::shared_ptr<CmVKPipeline> mPipeline;

        std::shared_ptr<CmVKDescriptorPool> mDescriptorPool;
        std::vector<std::shared_ptr<CmVKDescriptorPool>> mMaterialDescriptorPools;

        std::vector<VkDescriptorSet> mFrameUboDescSets;
        std::vector<std::shared_ptr<CmVKBuffer>> mFrameUboBuffers;

        std::vector<uint32_t> mMaterialDescriptorSetCounts;
        std::vector<std::vector<VkDescriptorSet>> mMaterialDescSets;
        std::vector<std::vector<VkDescriptorSet>> mMaterialResourceDescSets;
        std::vector<std::vector<std::shared_ptr<CmVKBuffer>>> mMaterialBuffers;
        std::vector<std::vector<uint64_t>> mMaterialParamVersions;
        std::vector<std::vector<uint64_t>> mMaterialResourceVersions;
        std::vector<std::vector<bool>> mMaterialResourceValid;
    };
}
