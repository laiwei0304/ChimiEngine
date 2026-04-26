#pragma once

#include "Graphic/CmVKCommon.h"

namespace chimi
{
    class CmVKDevice;
    class CmVKRenderPass;
    class CmVKImageView;
    class CmVKImage;

    class CmVKFrameBuffer{
    public:
        CmVKFrameBuffer(CmVKDevice *device, CmVKRenderPass *renderPass, const std::vector<std::shared_ptr<CmVKImage>> &images, uint32_t width, uint32_t height);
        ~CmVKFrameBuffer();

        bool ReCreate(const std::vector<std::shared_ptr<CmVKImage>> &images, uint32_t width, uint32_t height);

        VkFramebuffer GetHandle() const { return mHandle; }
        uint32_t GetWidth() const { return mWidth; }
        uint32_t GetHeight() const { return mHeight; }
    private:
        VkFramebuffer mHandle = VK_NULL_HANDLE;
        CmVKDevice *mDevice;
        CmVKRenderPass *mRenderPass;
        uint32_t mWidth;
        uint32_t mHeight;
        std::vector<std::shared_ptr<CmVKImage>> mImages;
        std::vector<std::shared_ptr<CmVKImageView>> mImageViews;
    };
}
