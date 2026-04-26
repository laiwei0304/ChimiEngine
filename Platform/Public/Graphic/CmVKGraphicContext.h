#pragma once

#include "CmGraphicContext.h"
#include "CmVKCommon.h"

namespace chimi
{
    struct QueueFamilyInfo{
        int32_t queueFamilyIndex = -1;
        uint32_t queueCount;
    };

    class CmVKGraphicContext : public CmGraphicContext{
    public:
        CmVKGraphicContext(CmWindow *window);
        ~CmVKGraphicContext() override;

        VkInstance GetInstance() const { return mInstance; }
        VkSurfaceKHR GetSurface() const { return mSurface; }
        VkPhysicalDevice GetPhyDevice() const { return mPhyDevice; }
        const QueueFamilyInfo &GetGraphicQueueFamilyInfo() const { return mGraphicQueueFamily; }
        const QueueFamilyInfo &GetPresentQueueFamilyInfo() const { return mPresentQueueFamily; }
        VkPhysicalDeviceMemoryProperties GetPhyDeviceMemProperties() const { return mPhyDeviceMemProperties; }
        bool IsSameGraphicPresentQueueFamily() const { return mGraphicQueueFamily.queueFamilyIndex == mPresentQueueFamily.queueFamilyIndex; }
    private:
        static void PrintPhyDeviceInfo(VkPhysicalDeviceProperties &props);
        static uint32_t GetPhyDeviceScore(VkPhysicalDeviceProperties &props);

        void CreateInstance();
        void CreateSurface(CmWindow *window);
        void SelectPhyDevice();

        VkInstance mInstance;
        VkSurfaceKHR mSurface;

        VkPhysicalDevice mPhyDevice;
        QueueFamilyInfo mGraphicQueueFamily;
        QueueFamilyInfo mPresentQueueFamily;
        VkPhysicalDeviceMemoryProperties mPhyDeviceMemProperties;
    };
}
