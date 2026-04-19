#include "rhi/vulkan/VulkanInstance.h"

#include "core/Assert.h"
#include "platform/Window.h"
#include "rhi/vulkan/VulkanFrameRecording.h"
#include "rhi/vulkan/VulkanMeshPipeline.h"

#include <stdexcept>
#include <vector>

#include <GLFW/glfw3.h>
#include <volk.h>

#include "spdlog/spdlog.h"

namespace chimi::rhi::vulkan
{
VulkanInstance::VulkanInstance(const chimi::platform::Window& window)
    : m_window(&window)
{
    VK_CHECK(volkInitialize());
    chimi::rhi::vulkan::CreateInstance(m_context);
    volkLoadInstance(m_context.instance);

    chimi::rhi::vulkan::CreateSurface(m_context, window);
    spdlog::info("Created Vulkan surface");

    chimi::rhi::vulkan::PickPhysicalDevice(m_context);
    chimi::rhi::vulkan::CreateLogicalDevice(m_context);
    chimi::rhi::vulkan::CreateUploadContext(m_upload, m_context);
    chimi::rhi::vulkan::CreateSwapchain(m_swapchain, m_context, window);
    chimi::rhi::vulkan::CreateSwapchainImageViews(m_swapchain, m_context.device);
    chimi::rhi::vulkan::CreateDepthResources(
        m_swapchain,
        m_context,
        chimi::rhi::vulkan::FindSupportedFormat,
        [this](uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            return chimi::rhi::vulkan::FindMemoryType(m_context.physicalDevice, typeFilter, properties);
        });
    chimi::rhi::vulkan::CreateFrameContexts(
        m_frameContexts,
        m_context.device,
        m_context.queueFamilyIndices.graphicsFamily.value(),
        kFramesInFlight);
    chimi::rhi::vulkan::CreateSwapchainSemaphores(m_swapchain, m_context.device);

    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(m_context.instance, &physicalDeviceCount, nullptr));

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    if (physicalDeviceCount > 0)
    {
        VK_CHECK(vkEnumeratePhysicalDevices(m_context.instance, &physicalDeviceCount, physicalDevices.data()));
    }

    spdlog::info("Created Vulkan instance");
    spdlog::info("Vulkan physical devices available: {}", physicalDeviceCount);

    for (const VkPhysicalDevice physicalDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        spdlog::info(
            "Detected GPU: {} | API {}.{}.{} | Driver {}",
            properties.deviceName,
            VK_API_VERSION_MAJOR(properties.apiVersion),
            VK_API_VERSION_MINOR(properties.apiVersion),
            VK_API_VERSION_PATCH(properties.apiVersion),
            properties.driverVersion);
    }

    if (ValidationEnabled())
    {
        spdlog::info("Validation layers requested for this debug build");
    }
    else
    {
        spdlog::info("Validation layers are disabled for this build");
    }

    spdlog::info(
        "Selected queue families | graphics={} present={}",
        m_context.queueFamilyIndices.graphicsFamily.value(),
        m_context.queueFamilyIndices.presentFamily.value());
    spdlog::info(
        "Created swapchain | images={} format={} extent={}x{}",
        m_swapchain.images.size(),
        static_cast<int>(m_swapchain.imageFormat),
        m_swapchain.extent.width,
        m_swapchain.extent.height);
}

VulkanInstance::~VulkanInstance()
{
    if (m_context.device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_context.device);
    }

    chimi::rhi::vulkan::DestroyFrameContexts(m_frameContexts, m_context.device);
    chimi::rhi::vulkan::DestroySwapchainSemaphores(m_swapchain, m_context.device);
    DestroyMeshPass(m_meshPass, m_context.device);
    DestroyMeshCache(m_meshCache, [this](MeshBuffer& meshBuffer)
    {
        chimi::rhi::vulkan::DestroyMeshBuffer(m_context.device, meshBuffer);
    });
    chimi::rhi::vulkan::CleanupSwapchain(m_swapchain, m_context.device);
    chimi::rhi::vulkan::DestroyUploadContext(m_upload, m_context.device);
    chimi::rhi::vulkan::DestroyContext(m_context);
}

VkInstance VulkanInstance::GetHandle() const { return m_context.instance; }
VkSurfaceKHR VulkanInstance::GetSurface() const { return m_context.surface; }
VkPhysicalDevice VulkanInstance::GetPhysicalDevice() const { return m_context.physicalDevice; }
VkDevice VulkanInstance::GetDevice() const { return m_context.device; }
VkQueue VulkanInstance::GetGraphicsQueue() const { return m_context.graphicsQueue; }
VkQueue VulkanInstance::GetPresentQueue() const { return m_context.presentQueue; }
const QueueFamilyIndices& VulkanInstance::GetQueueFamilyIndices() const { return m_context.queueFamilyIndices; }
VkSwapchainKHR VulkanInstance::GetSwapchain() const { return m_swapchain.handle; }
VkFormat VulkanInstance::GetSwapchainImageFormat() const { return m_swapchain.imageFormat; }
VkExtent2D VulkanInstance::GetSwapchainExtent() const { return m_swapchain.extent; }
const std::vector<VkImage>& VulkanInstance::GetSwapchainImages() const { return m_swapchain.images; }
const std::vector<VkImageView>& VulkanInstance::GetSwapchainImageViews() const { return m_swapchain.imageViews; }

void VulkanInstance::DrawFrame(const chimi::renderer::RenderPacket& renderPacket)
{
    SyncMeshPassPipeline(
        m_meshPass,
        m_context.device,
        m_swapchain.imageFormat,
        m_swapchain.depthFormat,
        renderPacket.mainMeshPass.pipeline);
    SyncMeshCache(
        m_meshCache,
        renderPacket.mainMeshPass,
        [this](const chimi::renderer::CpuMeshData& meshData) -> MeshBuffer
        {
            return chimi::rhi::vulkan::CreateMeshBuffer(
                m_context,
                m_upload,
                meshData,
                [this](uint32_t typeFilter, VkMemoryPropertyFlags properties)
                {
                    return chimi::rhi::vulkan::FindMemoryType(m_context.physicalDevice, typeFilter, properties);
                });
        },
        [this](MeshBuffer& meshBuffer)
        {
            chimi::rhi::vulkan::DestroyMeshBuffer(m_context.device, meshBuffer);
        });
    const PreparedMeshPass preparedMeshPass = BuildPreparedMeshPass(renderPacket, m_meshCache);
    FrameContext& frame = chimi::rhi::vulkan::GetCurrentFrame(m_frameContexts, m_currentFrameIndex);

    WaitForFrame(frame, m_context.device);

    const AcquireFrameResult acquireFrameResult =
        AcquireFrameImage(m_context.device, m_swapchain.handle, frame);
    const VkResult acquireResult = acquireFrameResult.result;
    const uint32_t imageIndex = acquireFrameResult.imageIndex;

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        HandleResize();
        return;
    }

    CHIMI_ASSERT(
        acquireResult == VK_SUCCESS || acquireResult == VK_SUBOPTIMAL_KHR,
        "Failed to acquire the next swapchain image");

    ResetFrameCommandPool(m_context.device, frame);
    chimi::rhi::vulkan::RecordFrameCommandBuffer(
        frame,
        imageIndex,
        m_swapchain,
        m_meshPass,
        preparedMeshPass);

    const VkSemaphore renderFinishedSemaphore = m_swapchain.renderFinishedSemaphores[imageIndex];
    SubmitFrame(m_context.graphicsQueue, frame, renderFinishedSemaphore);
    const VkResult presentResult =
        PresentFrame(m_context.presentQueue, m_swapchain.handle, imageIndex, renderFinishedSemaphore);
    CHIMI_ASSERT(
        presentResult == VK_SUCCESS
            || presentResult == VK_SUBOPTIMAL_KHR
            || presentResult == VK_ERROR_OUT_OF_DATE_KHR,
        "Failed to present the swapchain image");

    if (presentResult == VK_SUBOPTIMAL_KHR || presentResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        HandleResize();
    }

    VK_CHECK(vkQueueWaitIdle(m_context.presentQueue));
    m_currentFrameIndex = AdvanceFrameIndex(m_currentFrameIndex, kFramesInFlight);
}

void VulkanInstance::HandleResize()
{
    RecreateSwapchain();
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

    VK_CHECK(vkDeviceWaitIdle(m_context.device));
    chimi::rhi::vulkan::RebuildSwapchainResources(
        m_swapchain,
        m_context,
        *m_window,
        chimi::rhi::vulkan::FindSupportedFormat,
        [this](uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            return chimi::rhi::vulkan::FindMemoryType(m_context.physicalDevice, typeFilter, properties);
        });

    const bool rebuildMeshPass = m_meshPass.pipelineState.valid;
    const chimi::renderer::MeshPassPipelineConfig pipelineConfig = m_meshPass.pipelineState.config;
    DestroyMeshPass(m_meshPass, m_context.device);
    if (rebuildMeshPass)
    {
        SyncMeshPassPipeline(
            m_meshPass,
            m_context.device,
            m_swapchain.imageFormat,
            m_swapchain.depthFormat,
            pipelineConfig);
    }

    spdlog::info(
        "Recreated swapchain | images={} format={} extent={}x{}",
        m_swapchain.images.size(),
        static_cast<int>(m_swapchain.imageFormat),
        m_swapchain.extent.width,
        m_swapchain.extent.height);
}

}
