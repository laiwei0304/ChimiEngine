#pragma once

#include "CmRenderContext.h"

namespace chimi{
#define RENDERER_NUM_BUFFER     2

    class CmRenderer{
    public:
        CmRenderer();
        ~CmRenderer();

        bool Begin(int32_t *outImageIndex);
        bool End(int32_t imageIndex, const std::vector<VkCommandBuffer> &cmdBuffers);

        uint32_t GetCurrentBufferIndex() const { return mCurrentBuffer; }
        uint32_t GetBufferCount() const { return RENDERER_NUM_BUFFER; }
    private:
        void EnsureSubmittedSemaphores(uint32_t imageCount);

        uint32_t mCurrentBuffer = 0;
        std::vector<VkSemaphore> mImageAvailableSemaphores;
        std::vector<VkSemaphore> mSubmitedSemaphores;
        std::vector<VkFence> mFrameFences;
    };
}
