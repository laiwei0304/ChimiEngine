#pragma once

#include <cstdint>
#include <vector>

#include "rhi/vulkan/VulkanContext.h"
#include "rhi/vulkan/VulkanFrameContext.h"
#include "rhi/vulkan/VulkanMeshCache.h"
#include "rhi/vulkan/VulkanMeshPass.h"
#include "rhi/vulkan/VulkanSwapchain.h"
#include "rhi/vulkan/VulkanUploadContext.h"

#include <vulkan/vulkan.h>

namespace chimi::platform
{
class Window;
}

namespace chimi::renderer
{
struct RenderPacket;
}

namespace chimi::rhi::vulkan
{
class VulkanInstance
{
public:
    explicit VulkanInstance(const chimi::platform::Window& window);
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    VulkanInstance(VulkanInstance&&) = delete;
    VulkanInstance& operator=(VulkanInstance&&) = delete;

    VkInstance GetHandle() const;
    VkSurfaceKHR GetSurface() const;
    VkPhysicalDevice GetPhysicalDevice() const;
    VkDevice GetDevice() const;
    VkQueue GetGraphicsQueue() const;
    VkQueue GetPresentQueue() const;
    const QueueFamilyIndices& GetQueueFamilyIndices() const;
    VkSwapchainKHR GetSwapchain() const;
    VkFormat GetSwapchainImageFormat() const;
    VkExtent2D GetSwapchainExtent() const;
    const std::vector<VkImage>& GetSwapchainImages() const;
    const std::vector<VkImageView>& GetSwapchainImageViews() const;
    void DrawFrame(const chimi::renderer::RenderPacket& renderPacket);
    void HandleResize();

private:
    static constexpr uint32_t kFramesInFlight = 2;

    void RecreateSwapchain();

    const chimi::platform::Window* m_window = nullptr;
    VulkanContext m_context{};
    VulkanSwapchain m_swapchain{};
    VulkanUploadContext m_upload{};
    VulkanMeshCache m_meshCache{};
    VulkanMeshPass m_meshPass{};
    std::vector<FrameContext> m_frameContexts;
    uint32_t m_currentFrameIndex = 0;
};
}
