#include "rhi/vulkan/VulkanInstance.h"

#include "core/Assert.h"

#include <array>
#include <set>
#include <string_view>
#include <vector>

#include <volk.h>

#include "spdlog/spdlog.h"

namespace chimi::rhi::vulkan
{
namespace
{
constexpr std::array<const char*, 1> kRequiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
}

QueueFamilyIndices VulkanInstance::FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (uint32_t familyIndex = 0; familyIndex < queueFamilyCount; ++familyIndex)
    {
        const VkQueueFamilyProperties& queueFamily = queueFamilies[familyIndex];

        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0U)
        {
            indices.graphicsFamily = familyIndex;
        }

        VkBool32 supportsPresent = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &supportsPresent));
        if (supportsPresent == VK_TRUE)
        {
            indices.presentFamily = familyIndex;
        }

        if (indices.IsComplete())
        {
            break;
        }
    }

    return indices;
}

bool VulkanInstance::SupportsRequiredDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount = 0;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    if (extensionCount > 0)
    {
        VK_CHECK(vkEnumerateDeviceExtensionProperties(
            physicalDevice,
            nullptr,
            &extensionCount,
            availableExtensions.data()));
    }

    std::set<std::string_view> requiredExtensions(
        kRequiredDeviceExtensions.begin(),
        kRequiredDeviceExtensions.end());

    for (const VkExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool VulkanInstance::SupportsDynamicRendering(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;

    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext = &dynamicRenderingFeatures;

    vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
    return dynamicRenderingFeatures.dynamicRendering == VK_TRUE;
}

void VulkanInstance::PickPhysicalDevice()
{
    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr));
    CHIMI_ASSERT(physicalDeviceCount > 0, "No Vulkan physical devices were found");

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data()));

    for (const VkPhysicalDevice physicalDevice : physicalDevices)
    {
        QueueFamilyIndices queueFamilies = FindQueueFamilies(physicalDevice, m_surface);
        if (!queueFamilies.IsComplete())
        {
            continue;
        }

        if (!SupportsRequiredDeviceExtensions(physicalDevice))
        {
            continue;
        }

        if (!SupportsDynamicRendering(physicalDevice))
        {
            continue;
        }

        const SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(physicalDevice, m_surface);
        if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty())
        {
            continue;
        }

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        m_physicalDevice = physicalDevice;
        m_queueFamilyIndices = queueFamilies;

        spdlog::info("Selected physical device: {}", properties.deviceName);
        return;
    }

    throw std::runtime_error("Failed to find a suitable Vulkan physical device");
}

void VulkanInstance::CreateLogicalDevice()
{
    CHIMI_ASSERT(m_physicalDevice != VK_NULL_HANDLE, "Physical device must be selected before creating a logical device");

    const float queuePriority = 1.0f;
    std::set<uint32_t> uniqueQueueFamilies = {
        m_queueFamilyIndices.graphicsFamily.value(),
        m_queueFamilyIndices.presentFamily.value()
    };

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());

    for (const uint32_t queueFamilyIndex : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &dynamicRenderingFeatures;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(kRequiredDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = kRequiredDeviceExtensions.data();

    const auto validationLayers = BuildValidationLayers();
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.empty() ? nullptr : validationLayers.data();

    VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
    volkLoadDevice(m_device);

    vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);

    spdlog::info("Created logical device");
}

void VulkanInstance::CreateFrameContexts()
{
    m_frameContexts.resize(kFramesInFlight);

    for (FrameContext& frame : m_frameContexts)
    {
        VkCommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolCreateInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily.value();
        VK_CHECK(vkCreateCommandPool(m_device, &poolCreateInfo, nullptr, &frame.commandPool));

        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = frame.commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;
        VK_CHECK(vkAllocateCommandBuffers(m_device, &allocateInfo, &frame.commandBuffer));

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &frame.imageAvailableSemaphore));

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &frame.inFlightFence));
    }
}

void VulkanInstance::DestroyFrameContexts()
{
    for (FrameContext& frame : m_frameContexts)
    {
        if (frame.inFlightFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(m_device, frame.inFlightFence, nullptr);
            frame.inFlightFence = VK_NULL_HANDLE;
        }

        if (frame.imageAvailableSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device, frame.imageAvailableSemaphore, nullptr);
            frame.imageAvailableSemaphore = VK_NULL_HANDLE;
        }

        if (frame.commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(m_device, frame.commandPool, nullptr);
            frame.commandPool = VK_NULL_HANDLE;
            frame.commandBuffer = VK_NULL_HANDLE;
        }
    }

    m_frameContexts.clear();
}

uint32_t VulkanInstance::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

    for (uint32_t memoryTypeIndex = 0; memoryTypeIndex < memoryProperties.memoryTypeCount; ++memoryTypeIndex)
    {
        const bool typeMatches = (typeFilter & (1U << memoryTypeIndex)) != 0U;
        const bool propertiesMatch =
            (memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & properties) == properties;

        if (typeMatches && propertiesMatch)
        {
            return memoryTypeIndex;
        }
    }

    throw std::runtime_error("Failed to find a suitable Vulkan memory type");
}
}
