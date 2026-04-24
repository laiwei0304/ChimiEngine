#pragma once

#include "Graphic/CmVKFrameBuffer.h"
#include "Render/CmRenderContext.h"
#include "ECS/System/CmMaterialSystem.h"
#include "ECS/CmEntity.h"

namespace chimi{
    class CmRenderTarget{
    public:
        CmRenderTarget(CmVKRenderPass *renderPass);
        CmRenderTarget(CmVKRenderPass *renderPass, uint32_t bufferCount, VkExtent2D extent);
        ~CmRenderTarget();

        void Begin(VkCommandBuffer cmdBuffer);
        void End(VkCommandBuffer cmdBuffer);

        CmVKRenderPass *GetRenderPass() const { return mRenderPass; }
        CmVKFrameBuffer *GetFrameBuffer() const { return mFrameBuffers[mCurrentBufferIdx].get(); }

        void SetExtent(const VkExtent2D &extent);
        void SetBufferCount(uint32_t bufferCount);

        void SetColorClearValue(VkClearColorValue colorClearValue);
        void SetColorClearValue(uint32_t attachmentIndex, VkClearColorValue colorClearValue);
        void SetDepthStencilClearValue(VkClearDepthStencilValue depthStencilValue);
        void SetDepthStencilClearValue(uint32_t attachmentIndex, VkClearDepthStencilValue depthStencilValue);

        template<typename T, typename... Args>
        void CmdMaterialSystem(Args&&... args) {
            std::shared_ptr<CmMaterialSystem> system = std::make_shared<T>(std::forward<Args>(args)...);
            system->OnInit(mRenderPass);
            mMaterialSystemList.push_back(system);
        }

        void RenderMaterialSystems(VkCommandBuffer cmdBuffer) {
            for (auto &item: mMaterialSystemList){
                item->OnRender(cmdBuffer, this);
            }
        }

        void SetCamera(CmEntity *camera) { mCamera = camera; }
        CmEntity *GetCamera() const { return mCamera; }
    private:
        void Init();
        void ReCreate();

        std::vector<std::shared_ptr<CmVKFrameBuffer>> mFrameBuffers;

        CmVKRenderPass *mRenderPass;
        std::vector<VkClearValue> mClearValues;
        uint32_t mBufferCount;
        uint32_t mCurrentBufferIdx = 0;
        VkExtent2D mExtent;

        bool bSwapchainTarget = false;
        bool bBeginTarget = false;

        std::vector<std::shared_ptr<CmMaterialSystem>> mMaterialSystemList;
        CmEntity *mCamera = nullptr;

        bool bShouldUpdate = false;
    };
}
