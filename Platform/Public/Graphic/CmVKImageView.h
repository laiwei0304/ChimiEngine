#pragma once

#include "Graphic/CmVKCommon.h"

namespace chimi
{
    class CmVKDevice;

    class CmVKImageView{
    public:
        CmVKImageView(CmVKDevice *device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
        ~CmVKImageView();

        VkImageView GetHandle() const { return mHandle; }
    private:
        VkImageView mHandle = VK_NULL_HANDLE;

        CmVKDevice *mDevice;
    };
}
