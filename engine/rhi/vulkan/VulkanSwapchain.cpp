#include "rhi/vulkan/VulkanSwapchain.h"

#include "core/Assert.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <volk.h>

namespace chimi::rhi::vulkan
{
namespace
{
SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    SwapchainSupportDetails details{};

    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities));

    uint32_t formatCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
    if (formatCount > 0)
    {
        details.formats.resize(formatCount);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &formatCount,
            details.formats.data()));
    }

    uint32_t presentModeCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
    if (presentModeCount > 0)
    {
        details.presentModes.resize(presentModeCount);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &presentModeCount,
            details.presentModes.data()));
    }

    return details;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    CHIMI_ASSERT(!availableFormats.empty(), "Surface did not report any swapchain formats");
    return availableFormats.front();
}

VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const VkPresentModeKHR availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D desiredExtent)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    VkExtent2D actualExtent = desiredExtent;
    actualExtent.width = std::clamp(
        actualExtent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(
        actualExtent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);

    return actualExtent;
}
}

void CreateSwapchain(
    VulkanSwapchain& swapchain,
    const VulkanContext& context,
    const chimi::platform::Window& window)
{
    const SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(context.physicalDevice, context.surface);

    const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
    const VkPresentModeKHR presentMode = ChoosePresentMode(swapchainSupport.presentModes);
    const VkExtent2D extent = ChooseSwapExtent(
        swapchainSupport.capabilities,
        window.GetFramebufferExtent());

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0
        && imageCount > swapchainSupport.capabilities.maxImageCount)
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const uint32_t queueFamilyIndices[] = {
        context.queueFamilyIndices.graphicsFamily.value(),
        context.queueFamilyIndices.presentFamily.value()
    };

    if (context.queueFamilyIndices.graphicsFamily != context.queueFamilyIndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(context.device, &createInfo, nullptr, &swapchain.handle));

    VK_CHECK(vkGetSwapchainImagesKHR(context.device, swapchain.handle, &imageCount, nullptr));
    swapchain.images.resize(imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(context.device, swapchain.handle, &imageCount, swapchain.images.data()));

    swapchain.imageFormat = surfaceFormat.format;
    swapchain.extent = extent;
    swapchain.imageLayouts.assign(swapchain.images.size(), VK_IMAGE_LAYOUT_UNDEFINED);
}

void CreateSwapchainImageViews(VulkanSwapchain& swapchain, VkDevice device)
{
    swapchain.imageViews.reserve(swapchain.images.size());

    for (VkImage image : swapchain.images)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchain.imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView imageView = VK_NULL_HANDLE;
        VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &imageView));
        swapchain.imageViews.push_back(imageView);
    }
}

void CreateDepthResources(
    VulkanSwapchain& swapchain,
    const VulkanContext& context,
    const FindSupportedFormatFn& findSupportedFormat,
    const SwapchainFindMemoryTypeFn& findMemoryType)
{
    swapchain.depthFormat = findSupportedFormat(
        context.physicalDevice,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = swapchain.extent.width;
    imageCreateInfo.extent.height = swapchain.extent.height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = swapchain.depthFormat;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(context.device, &imageCreateInfo, nullptr, &swapchain.depthImage.image));

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(context.device, swapchain.depthImage.image, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(
        memoryRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(context.device, &allocateInfo, nullptr, &swapchain.depthImage.memory));
    VK_CHECK(vkBindImageMemory(context.device, swapchain.depthImage.image, swapchain.depthImage.memory, 0));

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = swapchain.depthImage.image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = swapchain.depthFormat;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    VK_CHECK(vkCreateImageView(context.device, &imageViewCreateInfo, nullptr, &swapchain.depthImage.imageView));

    swapchain.depthImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void RebuildSwapchainResources(
    VulkanSwapchain& swapchain,
    const VulkanContext& context,
    const chimi::platform::Window& window,
    const FindSupportedFormatFn& findSupportedFormat,
    const SwapchainFindMemoryTypeFn& findMemoryType)
{
    CleanupSwapchain(swapchain, context.device);
    CreateSwapchain(swapchain, context, window);
    CreateSwapchainImageViews(swapchain, context.device);
    CreateDepthResources(swapchain, context, findSupportedFormat, findMemoryType);
    DestroySwapchainSemaphores(swapchain, context.device);
    CreateSwapchainSemaphores(swapchain, context.device);
}

void CreateSwapchainSemaphores(VulkanSwapchain& swapchain, VkDevice device)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    swapchain.renderFinishedSemaphores.resize(swapchain.images.size(), VK_NULL_HANDLE);
    for (VkSemaphore& semaphore : swapchain.renderFinishedSemaphores)
    {
        VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));
    }
}

void DestroySwapchainSemaphores(VulkanSwapchain& swapchain, VkDevice device)
{
    for (VkSemaphore semaphore : swapchain.renderFinishedSemaphores)
    {
        if (semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
    }

    swapchain.renderFinishedSemaphores.clear();
}

void CleanupSwapchain(VulkanSwapchain& swapchain, VkDevice device)
{
    if (swapchain.depthImage.imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, swapchain.depthImage.imageView, nullptr);
        swapchain.depthImage.imageView = VK_NULL_HANDLE;
    }

    if (swapchain.depthImage.image != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, swapchain.depthImage.image, nullptr);
        swapchain.depthImage.image = VK_NULL_HANDLE;
    }

    if (swapchain.depthImage.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, swapchain.depthImage.memory, nullptr);
        swapchain.depthImage.memory = VK_NULL_HANDLE;
    }

    for (VkImageView imageView : swapchain.imageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    swapchain.imageViews.clear();
    swapchain.images.clear();
    swapchain.imageLayouts.clear();

    if (swapchain.handle != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device, swapchain.handle, nullptr);
        swapchain.handle = VK_NULL_HANDLE;
    }

    swapchain.imageFormat = VK_FORMAT_UNDEFINED;
    swapchain.extent = {};
    swapchain.depthFormat = VK_FORMAT_UNDEFINED;
    swapchain.depthImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}
}
