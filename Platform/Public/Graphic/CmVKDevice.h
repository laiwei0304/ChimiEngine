#pragma once

#include "CmVKCommon.h"

namespace chimi{
    class CmVKGraphicContext;
    class CmVKQueue;
    class CmVKCommandPool;

    struct CmVkSettings{
        VkFormat surfaceFormat = VK_FORMAT_B8G8R8A8_UNORM;
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        uint32_t swapchainImageCount = 3;
    };

    class CmVKDevice{
    public:
        CmVKDevice(CmVKGraphicContext *context, uint32_t graphicQueueCount, uint32_t presentQueueCount, const CmVkSettings &settings = {});
        ~CmVKDevice();

        VkDevice GetHandle() const { return mHandle; }

        const CmVkSettings &GetSettings() const { return mSettings; }
        VkPipelineCache GetPipelineCache() const { return mPipelineCache; }

        CmVKQueue* GetGraphicQueue(uint32_t index) const { return mGraphicQueues.size() < index + 1 ? nullptr : mGraphicQueues[index].get(); };
        CmVKQueue* GetFirstGraphicQueue() const { return mGraphicQueues.empty() ? nullptr : mGraphicQueues[0].get(); };
        CmVKQueue* GetPresentQueue(uint32_t index) const { return mPresentQueues.size() < index + 1 ? nullptr : mPresentQueues[index].get(); };
        CmVKQueue* GetFirstPresentQueue() const { return mPresentQueues.empty() ? nullptr : mPresentQueues[0].get(); };
        CmVKCommandPool *GetDefaultCmdPool() const { return mDefaultCmdPool.get(); }

        int32_t GetMemoryIndex(VkMemoryPropertyFlags memProps, uint32_t memoryTypeBits) const;
        VkCommandBuffer CreateAndBeginOneCmdBuffer();
        void SubmitOneCmdBuffer(VkCommandBuffer cmdBuffer);

        VkResult CreateSimpleSampler(VkFilter filter, VkSamplerAddressMode addressMode, VkSampler *outSampler);
    private:
        void CreatePipelineCache();
        void CreateDefaultCmdPool();

        VkDevice mHandle = VK_NULL_HANDLE;
        CmVKGraphicContext *mContext;

        std::vector<std::shared_ptr<CmVKQueue>> mGraphicQueues;
        std::vector<std::shared_ptr<CmVKQueue>> mPresentQueues;
        std::shared_ptr<CmVKCommandPool> mDefaultCmdPool;

        CmVkSettings mSettings;

        VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
    };
}