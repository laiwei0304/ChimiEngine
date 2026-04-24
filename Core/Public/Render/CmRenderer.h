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
    private:
        uint32_t mCurrentBuffer = 0;
        std::vector<VkSemaphore> mImageAvailableSemaphores;
        std::vector<VkSemaphore> mSubmitedSemaphores;
        std::vector<VkFence> mFrameFences;
    };
}
