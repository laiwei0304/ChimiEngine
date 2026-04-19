#include "rhi/vulkan/VulkanContext.h"

#include "core/Assert.h"
#include "platform/Window.h"

#include <array>
#include <set>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <GLFW/glfw3.h>
#include <volk.h>

#include "spdlog/spdlog.h"

namespace chimi::rhi::vulkan
{
namespace
{
constexpr const char* kApplicationName = "ChimiEngineSandbox";
constexpr const char* kEngineName = "ChimiEngine";
constexpr std::array<const char*, 1> kRequiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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

bool SupportsRequiredDeviceExtensions(VkPhysicalDevice physicalDevice)
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

bool SupportsDynamicRendering(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;

    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext = &dynamicRenderingFeatures;

    vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
    return dynamicRenderingFeatures.dynamicRendering == VK_TRUE;
}
}

bool QueueFamilyIndices::IsComplete() const
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

bool ValidationEnabled()
{
#if defined(NDEBUG)
    return false;
#else
#if CHIMI_ENABLE_VALIDATION
    return true;
#else
    return false;
#endif
#endif
}

std::vector<const char*> BuildRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    CHIMI_ASSERT(glfwExtensions != nullptr, "Failed to query required GLFW Vulkan extensions");

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ValidationEnabled())
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

std::vector<const char*> BuildValidationLayers()
{
    if (!ValidationEnabled())
    {
        return {};
    }

    return { "VK_LAYER_KHRONOS_validation" };
}

VkFormat FindSupportedFormat(
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

uint32_t FindMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

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

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
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

void CreateInstance(VulkanContext& context)
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = kApplicationName;
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.pEngineName = kEngineName;
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    const auto extensions = BuildRequiredExtensions();
    const auto validationLayers = BuildValidationLayers();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.empty() ? nullptr : validationLayers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &context.instance));
}

void CreateSurface(VulkanContext& context, const chimi::platform::Window& window)
{
    context.surface = window.CreateVulkanSurface(context.instance);
}

void PickPhysicalDevice(VulkanContext& context)
{
    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount, nullptr));
    CHIMI_ASSERT(physicalDeviceCount > 0, "No Vulkan physical devices were found");

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount, physicalDevices.data()));

    for (const VkPhysicalDevice physicalDevice : physicalDevices)
    {
        QueueFamilyIndices queueFamilies = FindQueueFamilies(physicalDevice, context.surface);
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

        const SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(physicalDevice, context.surface);
        if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty())
        {
            continue;
        }

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        context.physicalDevice = physicalDevice;
        context.queueFamilyIndices = queueFamilies;

        spdlog::info("Selected physical device: {}", properties.deviceName);
        return;
    }

    throw std::runtime_error("Failed to find a suitable Vulkan physical device");
}

void CreateLogicalDevice(VulkanContext& context)
{
    CHIMI_ASSERT(context.physicalDevice != VK_NULL_HANDLE, "Physical device must be selected before creating a logical device");

    const float queuePriority = 1.0f;
    std::set<uint32_t> uniqueQueueFamilies = {
        context.queueFamilyIndices.graphicsFamily.value(),
        context.queueFamilyIndices.presentFamily.value()
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

    VK_CHECK(vkCreateDevice(context.physicalDevice, &createInfo, nullptr, &context.device));
    volkLoadDevice(context.device);

    vkGetDeviceQueue(
        context.device,
        context.queueFamilyIndices.graphicsFamily.value(),
        0,
        &context.graphicsQueue);
    vkGetDeviceQueue(
        context.device,
        context.queueFamilyIndices.presentFamily.value(),
        0,
        &context.presentQueue);

    spdlog::info("Created logical device");
}

void DestroyContext(VulkanContext& context)
{
    if (context.device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(context.device, nullptr);
        context.device = VK_NULL_HANDLE;
    }

    if (context.surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
        context.surface = VK_NULL_HANDLE;
    }

    if (context.instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(context.instance, nullptr);
        context.instance = VK_NULL_HANDLE;
    }

    context.physicalDevice = VK_NULL_HANDLE;
    context.graphicsQueue = VK_NULL_HANDLE;
    context.presentQueue = VK_NULL_HANDLE;
    context.queueFamilyIndices = {};
}
}
