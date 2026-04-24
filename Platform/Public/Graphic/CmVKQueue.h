#pragma once

#include "CmVKCommon.h"

namespace chimi
{
    class CmVKQueue{
    public:
        CmVKQueue(uint32_t familyIndex, uint32_t index, VkQueue queue, bool canPresent);
        ~CmVKQueue() = default;

        void WaitIdle() const;
        VkQueue GetHandle() const { return mHandle; }

        void Submit(const std::vector<VkCommandBuffer> &cmdBuffers, const std::vector<VkSemaphore> &waitSemaphores = {}, const std::vector<VkSemaphore> &signalSemaphores = {}, VkFence frameFence = VK_NULL_HANDLE);
    private:
        uint32_t mFamilyIndex;
        uint32_t mIndex;
        VkQueue mHandle;
        bool canPresent;
    };;
}
