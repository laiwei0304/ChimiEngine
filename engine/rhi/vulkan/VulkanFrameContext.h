#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

namespace chimi::rhi::vulkan
{
struct FrameContext
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;
};

struct AcquireFrameResult
{
    VkResult result = VK_SUCCESS;
    uint32_t imageIndex = 0;
};

void CreateFrameContexts(
    std::vector<FrameContext>& frameContexts,
    VkDevice device,
    uint32_t queueFamilyIndex,
    uint32_t framesInFlight);
void DestroyFrameContexts(std::vector<FrameContext>& frameContexts, VkDevice device);
FrameContext& GetCurrentFrame(std::vector<FrameContext>& frameContexts, uint32_t currentFrameIndex);
void WaitForFrame(FrameContext& frame, VkDevice device);
AcquireFrameResult AcquireFrameImage(VkDevice device, VkSwapchainKHR swapchain, const FrameContext& frame);
void ResetFrameCommandPool(VkDevice device, const FrameContext& frame);
void SubmitFrame(
    VkQueue graphicsQueue,
    FrameContext& frame,
    VkSemaphore renderFinishedSemaphore);
VkResult PresentFrame(
    VkQueue presentQueue,
    VkSwapchainKHR swapchain,
    uint32_t imageIndex,
    VkSemaphore renderFinishedSemaphore);
uint32_t AdvanceFrameIndex(uint32_t currentFrameIndex, uint32_t framesInFlight);
}
