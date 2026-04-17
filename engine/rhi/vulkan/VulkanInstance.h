#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace chimi::rhi::vulkan
{
class VulkanInstance
{
public:
    VulkanInstance();
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    VulkanInstance(VulkanInstance&&) = delete;
    VulkanInstance& operator=(VulkanInstance&&) = delete;

    VkInstance GetHandle() const;

private:
    static std::vector<const char*> BuildRequiredExtensions();
    static std::vector<const char*> BuildValidationLayers();
    static bool ValidationEnabled();

    VkInstance m_instance = VK_NULL_HANDLE;
};
}
