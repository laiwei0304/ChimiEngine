#pragma once

#include "Graphic/CmVKCommon.h"

namespace chimi
{
    class CmVKGraphicContext;
    class CmVKDevice;

    struct SurfaceInfo{
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;
    };

    class CmVKSwapchain{
    public:
        CmVKSwapchain(CmVKGraphicContext *context, CmVKDevice *device);
        ~CmVKSwapchain();

        bool ReCreate();

        VkResult AcquireImage(int32_t *outImageIndex, VkSemaphore semaphore, VkFence fence = VK_NULL_HANDLE);
        VkResult Present(int32_t imageIndex, const std::vector<VkSemaphore> &waitSemaphores);

        const std::vector<VkImage> &GetImages() const { return mImages; }
        uint32_t GetWidth() const { return mSurfaceInfo.capabilities.currentExtent.width; }
        uint32_t GetHeight() const { return mSurfaceInfo.capabilities.currentExtent.height; }
        int32_t GetCurrentImageIndex() const { return mCurrentImageIndex; }

        const SurfaceInfo &GetSurfaceInfo() const { return mSurfaceInfo; }
    private:
        void SetupSurfaceCapabilities();

        VkSwapchainKHR mHandle = VK_NULL_HANDLE;

        CmVKGraphicContext *mContext;
        CmVKDevice *mDevice;
        std::vector<VkImage> mImages;

        int32_t mCurrentImageIndex = -1;

        SurfaceInfo mSurfaceInfo;
    };
}
