#pragma once

#include "CmVKCommon.h"

namespace chimi
{
    class CmVKDevice;

    class CmVKCommandPool{
    public:
        CmVKCommandPool(CmVKDevice *device, uint32_t queueFamilyIndex);
        ~CmVKCommandPool();

        static void BeginCommandBuffer(VkCommandBuffer cmdBuffer);
        static void EndCommandBuffer(VkCommandBuffer cmdBuffer);

        std::vector<VkCommandBuffer> AllocateCommandBuffer(uint32_t count) const;
        VkCommandBuffer AllocateOneCommandBuffer() const;
        VkCommandPool GetHandle() const { return mHandle; }
    private:
        VkCommandPool mHandle;

        CmVKDevice *mDevice;
    };
}
