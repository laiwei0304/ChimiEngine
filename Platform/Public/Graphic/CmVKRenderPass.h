#pragma once

#include "Graphic/CmVKCommon.h"

namespace chimi
{
    class CmVKDevice;
    class CmVKFrameBuffer;

    struct Attachment{
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    };

    struct RenderSubPass{
        std::vector<uint32_t> inputAttachments;
        std::vector<uint32_t> colorAttachments;
        std::vector<uint32_t> depthStencilAttachments;
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
    };

    class CmVKRenderPass{
    public:
        CmVKRenderPass(CmVKDevice *device, const std::vector<Attachment> &attachments = {}, const std::vector<RenderSubPass> &subPasses = {});
        ~CmVKRenderPass();

        VkRenderPass GetHandle() const { return mHandle; }

        void Begin(VkCommandBuffer cmdBuffer, CmVKFrameBuffer *frameBuffer, const std::vector<VkClearValue> &clearValues) const;
        void End(VkCommandBuffer cmdBuffer) const;

        const std::vector<Attachment> &GetAttachments() const { return mAttachments; }
        uint32_t GetAttachmentSize() const { return mAttachments.size(); }
        const std::vector<RenderSubPass> &GetSubPasses() const { return mSubPasses; }
    private:
        VkRenderPass mHandle = VK_NULL_HANDLE;
        CmVKDevice *mDevice;

        std::vector<Attachment> mAttachments;
        std::vector<RenderSubPass> mSubPasses;
    };
}
