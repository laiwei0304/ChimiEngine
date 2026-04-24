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
        void OnRender(VkCommandBuffer cmdBuffer, CmRenderTarget *renderTarget) override;
        void OnDestroy() override;
    private:
        void ReCreateMaterialDescPool(uint32_t materialCount);
        void UpdateFrameUboDescSet(CmRenderTarget *renderTarget);
        void UpdateMaterialParamsDescSet(VkDescriptorSet descSet, CmUnlitMaterial *material);
        void UpdateMaterialResourceDescSet(VkDescriptorSet descSet, CmUnlitMaterial *material);

        std::shared_ptr<CmVKDescriptorSetLayout> mFrameUboDescSetLayout;
        std::shared_ptr<CmVKDescriptorSetLayout> mMaterialParamDescSetLayout;
        std::shared_ptr<CmVKDescriptorSetLayout> mMaterialResourceDescSetLayout;

        std::shared_ptr<CmVKPipelineLayout> mPipelineLayout;
        std::shared_ptr<CmVKPipeline> mPipeline;

        std::shared_ptr<CmVKDescriptorPool> mDescriptorPool;
        std::shared_ptr<CmVKDescriptorPool> mMaterialDescriptorPool;

        VkDescriptorSet mFrameUboDescSet;
        std::shared_ptr<CmVKBuffer> mFrameUboBuffer;

        uint32_t mLastDescriptorSetCount = 0;
        std::vector<VkDescriptorSet> mMaterialDescSets;
        std::vector<VkDescriptorSet> mMaterialResourceDescSets;
        std::vector<std::shared_ptr<CmVKBuffer>> mMaterialBuffers;
    };
}
