#include "Graphic/CmVKFramebuffer.h"
#include "Graphic/CmVKDevice.h"
#include "Graphic/CmVKRenderPass.h"
#include "Graphic/CmVKImage.h"
#include "Graphic/CmVKImageView.h"

namespace chimi
{
    CmVKFrameBuffer::CmVKFrameBuffer(CmVKDevice *device, CmVKRenderPass *renderPass, const std::vector<std::shared_ptr<CmVKImage>> &images, uint32_t width, uint32_t height)
                                        : mDevice(device), mRenderPass(renderPass), mImages(images), mWidth(width), mHeight(height){
        ReCreate(images, width, height);
    }

    CmVKFrameBuffer::~CmVKFrameBuffer() {
        VK_D(Framebuffer, mDevice->GetHandle(), mHandle);
    }

    bool CmVKFrameBuffer::ReCreate(const std::vector<std::shared_ptr<CmVKImage>> &images, uint32_t width, uint32_t height) {
        VkResult ret;

        VK_D(Framebuffer, mDevice->GetHandle(), mHandle);

        mWidth = width;
        mHeight = height;
        mImageViews.clear();

        std::vector<VkImageView> attachments(images.size());
        for (int i = 0 ; i < images.size(); i++){
            bool isDepthFormat = IsDepthOnlyFormat(images[i]->GetFormat()); // FIXME when format is stencil format
            mImageViews.push_back(std::make_shared<CmVKImageView>(mDevice, images[i]->GetHandle(),
                                                                  images[i]->GetFormat(), isDepthFormat ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT));
            attachments[i] = mImageViews[i]->GetHandle();
        }

        VkFramebufferCreateInfo frameBufferInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = mRenderPass->GetHandle(),
                .attachmentCount = static_cast<uint32_t>(mImageViews.size()),
                .pAttachments = attachments.data(),
                .width = width,
                .height = height,
                .layers = 1
        };
        ret = vkCreateFramebuffer(mDevice->GetHandle(), &frameBufferInfo, nullptr, &mHandle);
        LOG_T("FrameBuffer {0}, new: {1}, width: {2}, height: {3}, view count: {4}", __FUNCTION__, (void*)mHandle, mWidth, mHeight, mImageViews.size());
        return ret == VK_SUCCESS;
    }
}
