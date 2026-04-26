#pragma once

#include "ECS/System/CmMaterialSystem.h"
#include "ECS/Component/CmTransformComponent.h"
#include "ECS/Component/Material/CmBaseMaterialComponent.h"
#include "CmGraphicContext.h"

namespace chimi{
    class CmVKPipelineLayout;
    class CmVKPipeline;

    struct PushConstants{
        glm::mat4 matrix{ 1.f };
        uint32_t colorType;
    };

    class CmBaseMaterialSystem : public CmMaterialSystem{
    public:
        void OnInit(CmVKRenderPass *renderPass) override;
        void OnRender(VkCommandBuffer cmdBuffer, CmRenderTarget *renderTarget, uint32_t frameIndex) override;
        void OnDestroy() override;
    private:
        std::shared_ptr<CmVKPipelineLayout> mPipelineLayout;
        std::shared_ptr<CmVKPipeline> mPipeline;
    };
}
