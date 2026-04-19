#pragma once

#include "platform/Window.h"
#include "rhi/vulkan/VulkanContext.h"

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>

namespace chimi::rhi::vulkan
{
struct AllocatedImage
{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
};

struct VulkanSwapchain
{
    VkSwapchainKHR handle = VK_NULL_HANDLE;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{};
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkImageLayout> imageLayouts;
    AllocatedImage depthImage{};
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout depthImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    std::vector<VkSemaphore> renderFinishedSemaphores;
};

using FindSupportedFormatFn = std::function<VkFormat(
    VkPhysicalDevice,
    const std::vector<VkFormat>&,
    VkImageTiling,
    VkFormatFeatureFlags)>;
using SwapchainFindMemoryTypeFn = std::function<uint32_t(uint32_t, VkMemoryPropertyFlags)>;

void CreateSwapchain(
    VulkanSwapchain& swapchain,
    const VulkanContext& context,
    const chimi::platform::Window& window);
void CreateSwapchainImageViews(VulkanSwapchain& swapchain, VkDevice device);
void CreateDepthResources(
    VulkanSwapchain& swapchain,
    const VulkanContext& context,
    const FindSupportedFormatFn& findSupportedFormat,
    const SwapchainFindMemoryTypeFn& findMemoryType);
void RebuildSwapchainResources(
    VulkanSwapchain& swapchain,
    const VulkanContext& context,
    const chimi::platform::Window& window,
    const FindSupportedFormatFn& findSupportedFormat,
    const SwapchainFindMemoryTypeFn& findMemoryType);
void CreateSwapchainSemaphores(VulkanSwapchain& swapchain, VkDevice device);
void DestroySwapchainSemaphores(VulkanSwapchain& swapchain, VkDevice device);
void CleanupSwapchain(VulkanSwapchain& swapchain, VkDevice device);
}
