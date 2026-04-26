#include "Render/CmRenderer.h"
#include "CmApplication.h"
#include "Graphic/CmVKQueue.h"

namespace chimi{
    CmRenderer::CmRenderer() {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();
        chimi::CmVKSwapchain *swapchain = renderCxt->GetSwapchain();

        mImageAvailableSemaphores.resize(RENDERER_NUM_BUFFER);
        mFrameFences.resize(RENDERER_NUM_BUFFER);
        VkSemaphoreCreateInfo semaphoreInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0
        };
        VkFenceCreateInfo fenceInfo = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT

        };
        for(int i = 0; i < RENDERER_NUM_BUFFER; i++){
            CALL_VK(vkCreateSemaphore(device->GetHandle(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]));
            CALL_VK(vkCreateFence(device->GetHandle(), &fenceInfo, nullptr, &mFrameFences[i]));
        }
        EnsureSubmittedSemaphores(swapchain->GetImages().size());
    }

    CmRenderer::~CmRenderer() {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();
        for (const auto &item: mImageAvailableSemaphores){
            VK_D(Semaphore, device->GetHandle(), item);
        }
        for (const auto &item: mSubmitedSemaphores){
            VK_D(Semaphore, device->GetHandle(), item);
        }
        for (const auto &item: mFrameFences){
            VK_D(Fence, device->GetHandle(), item);
        }
    }

    bool CmRenderer::Begin(int32_t *outImageIndex) {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();
        chimi::CmVKSwapchain *swapchain = renderCxt->GetSwapchain();

        bool bShouldUpdateTarget = false;

        CALL_VK(vkWaitForFences(device->GetHandle(), 1, &mFrameFences[mCurrentBuffer], VK_TRUE, UINT64_MAX));
        CALL_VK(vkResetFences(device->GetHandle(), 1, &mFrameFences[mCurrentBuffer]));

        VkResult ret = swapchain->AcquireImage(outImageIndex, mImageAvailableSemaphores[mCurrentBuffer]);
        if(ret == VK_ERROR_OUT_OF_DATE_KHR){
            CALL_VK(vkDeviceWaitIdle(device->GetHandle()));
            VkExtent2D originExtent = { swapchain->GetWidth(), swapchain->GetHeight() };
            bool bSuc = swapchain->ReCreate();
            if(bSuc){
                EnsureSubmittedSemaphores(swapchain->GetImages().size());
            }

            VkExtent2D newExtent = { swapchain->GetWidth(), swapchain->GetHeight() };
            if(bSuc && (originExtent.width != newExtent.width || originExtent.height != newExtent.height)){
                bShouldUpdateTarget = true;
            }
            ret = swapchain->AcquireImage(outImageIndex, mImageAvailableSemaphores[mCurrentBuffer]);
            if(ret != VK_SUCCESS && ret != VK_SUBOPTIMAL_KHR){
                LOG_E("Recreate swapchain error: {0}", vk_result_string(ret));
            }
        }
        return bShouldUpdateTarget;
    }

    bool CmRenderer::End(int32_t imageIndex, const std::vector<VkCommandBuffer> &cmdBuffers) {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();
        chimi::CmVKSwapchain *swapchain = renderCxt->GetSwapchain();
        bool bShouldUpdateTarget = false;

        VkSemaphore submittedSemaphore = mSubmitedSemaphores[imageIndex];
        device->GetFirstGraphicQueue()->Submit(cmdBuffers, { mImageAvailableSemaphores[mCurrentBuffer] }, { submittedSemaphore }, mFrameFences[mCurrentBuffer]);

        VkResult ret = swapchain->Present(imageIndex, { submittedSemaphore });
        if(ret == VK_SUBOPTIMAL_KHR || ret == VK_ERROR_OUT_OF_DATE_KHR){
            CALL_VK(vkDeviceWaitIdle(device->GetHandle()));
            VkExtent2D originExtent = { swapchain->GetWidth(), swapchain->GetHeight() };
            bool bSuc = swapchain->ReCreate();
            if(bSuc){
                EnsureSubmittedSemaphores(swapchain->GetImages().size());
            }

            VkExtent2D newExtent = { swapchain->GetWidth(), swapchain->GetHeight() };
            if(bSuc && (originExtent.width != newExtent.width || originExtent.height != newExtent.height)){
                bShouldUpdateTarget = true;
            }
        }
        mCurrentBuffer = (mCurrentBuffer + 1) % RENDERER_NUM_BUFFER;
        return bShouldUpdateTarget;
    }

    void CmRenderer::EnsureSubmittedSemaphores(uint32_t imageCount) {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();

        VkSemaphoreCreateInfo semaphoreInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0
        };

        uint32_t oldSize = static_cast<uint32_t>(mSubmitedSemaphores.size());
        if(oldSize < imageCount){
            mSubmitedSemaphores.resize(imageCount);
            for(uint32_t i = oldSize; i < imageCount; i++){
                CALL_VK(vkCreateSemaphore(device->GetHandle(), &semaphoreInfo, nullptr, &mSubmitedSemaphores[i]));
            }
            return;
        }

        for(uint32_t i = imageCount; i < oldSize; i++){
            VK_D(Semaphore, device->GetHandle(), mSubmitedSemaphores[i]);
        }
        mSubmitedSemaphores.resize(imageCount);
    }
}
