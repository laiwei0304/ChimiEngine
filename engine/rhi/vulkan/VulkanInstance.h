#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include "renderer/MeshData.h"
#include "renderer/Renderer.h"

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

struct FrameContext
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;
};

struct AllocatedBuffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct AllocatedImage
{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
};

struct UploadContext
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
};

struct MeshBuffer
{
    AllocatedBuffer vertexBuffer{};
    AllocatedBuffer indexBuffer{};
    uint32_t indexCount = 0;
};

class VulkanInstance
{
public:
    VulkanInstance(const chimi::platform::Window& window, const chimi::renderer::RenderFrameInput& frameInput);
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
    void DrawFrame();
    void HandleResize();

private:
    static constexpr uint32_t kFramesInFlight = 2;

    static std::vector<const char*> BuildRequiredExtensions();
    static std::vector<const char*> BuildValidationLayers();
    static bool ValidationEnabled();
    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static bool SupportsRequiredDeviceExtensions(VkPhysicalDevice physicalDevice);
    static SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D desiredExtent);
    static bool SupportsDynamicRendering(VkPhysicalDevice physicalDevice);
    static VkFormat FindSupportedFormat(
        VkPhysicalDevice physicalDevice,
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features);
    static void TransitionSwapchainImage(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);
    static void TransitionDepthImage(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);
    static std::vector<std::byte> ReadBinaryFile(const char* filePath);

    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain(const chimi::platform::Window& window);
    void CreateSwapchainImageViews();
    void CreateDepthResources();
    void CreateFrameContexts();
    void CreateSwapchainSemaphores();
    void RecordClearCommandBuffer(FrameContext& frame, uint32_t imageIndex);
    void CleanupSwapchain();
    void RecreateSwapchain();
    void DestroyFrameContexts();
    void DestroySwapchainSemaphores();
    FrameContext& GetCurrentFrame();
    void CreateSampleGeometryResources(const chimi::renderer::RenderFrameInput& frameInput);
    void CreateGraphicsPipeline();
    MeshBuffer CreateMeshBuffer(const chimi::renderer::CpuMeshData& meshData);
    void DrawMeshBuffer(VkCommandBuffer commandBuffer, const MeshBuffer& meshBuffer);
    void DestroyGraphicsPipeline();
    void DestroySampleGeometryResources();
    void CreateUploadContext();
    void DestroyUploadContext();
    AllocatedBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void DestroyBuffer(AllocatedBuffer& buffer);
    void UploadBufferWithStaging(const void* data, VkDeviceSize size, AllocatedBuffer& destinationBuffer, VkBufferUsageFlags usage);
    void CopyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);
    void ImmediateSubmit(const std::function<void(VkCommandBuffer)>& recordCommands);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    VkFormat FindDepthFormat() const;

    const chimi::platform::Window* m_window = nullptr;
    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapchainExtent{};
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkImageLayout> m_swapchainImageLayouts;
    AllocatedImage m_depthImage{};
    VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout m_depthImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<FrameContext> m_frameContexts;
    UploadContext m_uploadContext{};
    chimi::renderer::CameraData m_cameraData{};
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    MeshBuffer m_sampleMeshBuffer{};
    uint32_t m_currentFrameIndex = 0;
    QueueFamilyIndices m_queueFamilyIndices{};
};
}
