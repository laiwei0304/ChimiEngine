#pragma once

#include "Graphic/CmVKCommon.h"

namespace chimi
{
    class CmVKDevice;
    class CmVKBuffer;

    class CmVKImage{
    public:
        CmVKImage(CmVKDevice *device, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
        CmVKImage(CmVKDevice *device, VkImage image, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
        ~CmVKImage();

        static bool TransitionLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

        void CopyFromBuffer(VkCommandBuffer cmdBuffer, CmVKBuffer *buffer);

        VkFormat GetFormat() const { return mFormat; }
        VkImage GetHandle() const { return mHandle; }
    private:
        VkImage mHandle = VK_NULL_HANDLE;
        VkDeviceMemory mMemory  =VK_NULL_HANDLE;

        bool bCreateImage = true;

        CmVKDevice *mDevice;

        VkFormat mFormat;
        VkExtent3D mExtent;
        VkImageUsageFlags mUsage;
    };
}
