#pragma once

#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

namespace chimi::platform
{
class Window;
}

namespace chimi::rhi::vulkan
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() const;
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct VulkanContext
{
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    QueueFamilyIndices queueFamilyIndices{};
};

bool ValidationEnabled();
std::vector<const char*> BuildRequiredExtensions();
std::vector<const char*> BuildValidationLayers();
VkFormat FindSupportedFormat(
    VkPhysicalDevice physicalDevice,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features);
uint32_t FindMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties);
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
void CreateInstance(VulkanContext& context);
void CreateSurface(VulkanContext& context, const chimi::platform::Window& window);
void PickPhysicalDevice(VulkanContext& context);
void CreateLogicalDevice(VulkanContext& context);
void DestroyContext(VulkanContext& context);
}
