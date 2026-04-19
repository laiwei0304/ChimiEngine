#include "rhi/vulkan/VulkanInstance.h"

#include "core/Assert.h"
#include "platform/Window.h"

#include <algorithm>
#include <limits>

#include <volk.h>

#include "spdlog/spdlog.h"

namespace chimi::rhi::vulkan
{
VkFormat VulkanInstance::FindSupportedFormat(
    VkPhysicalDevice physicalDevice,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features)
{
    for (const VkFormat candidate : candidates)
    {
        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidate, &properties);

        const bool supportsFeatures = tiling == VK_IMAGE_TILING_LINEAR
            ? (properties.linearTilingFeatures & features) == features
            : (properties.optimalTilingFeatures & features) == features;

        if (supportsFeatures)
        {
            return candidate;
        }
    }

    throw std::runtime_error("Failed to find a supported Vulkan format");
}

SwapchainSupportDetails VulkanInstance::QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
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

VkSurfaceFormatKHR VulkanInstance::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR VulkanInstance::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D VulkanInstance::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D desiredExtent)
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

void VulkanInstance::CreateSwapchain(const chimi::platform::Window& window)
{
    const SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(m_physicalDevice, m_surface);

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
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const uint32_t queueFamilyIndices[] = {
        m_queueFamilyIndices.graphicsFamily.value(),
        m_queueFamilyIndices.presentFamily.value()
    };

    if (m_queueFamilyIndices.graphicsFamily != m_queueFamilyIndices.presentFamily)
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

    VK_CHECK(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr));
    m_swapchainImages.resize(imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data()));

    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
    m_swapchainImageLayouts.assign(m_swapchainImages.size(), VK_IMAGE_LAYOUT_UNDEFINED);
}

void VulkanInstance::CreateSwapchainImageViews()
{
    m_swapchainImageViews.reserve(m_swapchainImages.size());

    for (VkImage image : m_swapchainImages)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapchainImageFormat;
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
        VK_CHECK(vkCreateImageView(m_device, &createInfo, nullptr, &imageView));
        m_swapchainImageViews.push_back(imageView);
    }
}

void VulkanInstance::CreateDepthResources()
{
    m_depthFormat = FindDepthFormat();

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = m_swapchainExtent.width;
    imageCreateInfo.extent.height = m_swapchainExtent.height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = m_depthFormat;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(m_device, &imageCreateInfo, nullptr, &m_depthImage.image));

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(m_device, m_depthImage.image, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryType(
        memoryRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(m_device, &allocateInfo, nullptr, &m_depthImage.memory));
    VK_CHECK(vkBindImageMemory(m_device, m_depthImage.image, m_depthImage.memory, 0));

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = m_depthImage.image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = m_depthFormat;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    VK_CHECK(vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_depthImage.imageView));

    m_depthImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void VulkanInstance::CreateSwapchainSemaphores()
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    m_renderFinishedSemaphores.resize(m_swapchainImages.size(), VK_NULL_HANDLE);
    for (VkSemaphore& semaphore : m_renderFinishedSemaphores)
    {
        VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &semaphore));
    }
}

void VulkanInstance::DestroySwapchainSemaphores()
{
    for (VkSemaphore semaphore : m_renderFinishedSemaphores)
    {
        if (semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device, semaphore, nullptr);
        }
    }

    m_renderFinishedSemaphores.clear();
}

void VulkanInstance::CleanupSwapchain()
{
    if (m_depthImage.imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_device, m_depthImage.imageView, nullptr);
        m_depthImage.imageView = VK_NULL_HANDLE;
    }

    if (m_depthImage.image != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_device, m_depthImage.image, nullptr);
        m_depthImage.image = VK_NULL_HANDLE;
    }

    if (m_depthImage.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_device, m_depthImage.memory, nullptr);
        m_depthImage.memory = VK_NULL_HANDLE;
    }

    for (VkImageView imageView : m_swapchainImageViews)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    m_swapchainImageViews.clear();
    m_swapchainImages.clear();
    m_swapchainImageLayouts.clear();

    if (m_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }

    m_swapchainImageFormat = VK_FORMAT_UNDEFINED;
    m_swapchainExtent = {};
    m_depthFormat = VK_FORMAT_UNDEFINED;
    m_depthImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void VulkanInstance::RecreateSwapchain()
{
    CHIMI_ASSERT(m_window != nullptr, "Window must be available to recreate the swapchain");

    VkExtent2D extent = m_window->GetFramebufferExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        const_cast<chimi::platform::Window*>(m_window)->PollEvents();
        extent = m_window->GetFramebufferExtent();
    }

    VK_CHECK(vkDeviceWaitIdle(m_device));
    CleanupSwapchain();
    CreateSwapchain(*m_window);
    CreateSwapchainImageViews();
    CreateDepthResources();
    DestroySwapchainSemaphores();
    CreateSwapchainSemaphores();
    DestroyGraphicsPipeline();
    if (m_meshPipelineState.valid)
    {
        const chimi::renderer::MeshPassPipelineConfig pipelineConfig = m_meshPipelineState.config;
        CreateGraphicsPipeline(pipelineConfig);
        m_meshPipelineState.config = pipelineConfig;
        m_meshPipelineState.valid = true;
    }

    spdlog::info(
        "Recreated swapchain | images={} format={} extent={}x{}",
        m_swapchainImages.size(),
        static_cast<int>(m_swapchainImageFormat),
        m_swapchainExtent.width,
        m_swapchainExtent.height);
}
}
