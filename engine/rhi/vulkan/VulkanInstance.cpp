#include "rhi/vulkan/VulkanInstance.h"

#include "core/Assert.h"
#include "platform/Window.h"

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
}

bool QueueFamilyIndices::IsComplete() const
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

VulkanInstance::VulkanInstance(const chimi::platform::Window& window)
    : m_window(&window)
{
    VK_CHECK(volkInitialize());

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

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
    volkLoadInstance(m_instance);

    m_surface = window.CreateVulkanSurface(m_instance);
    spdlog::info("Created Vulkan surface");

    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateUploadContext();
    CreateSwapchain(window);
    CreateSwapchainImageViews();
    CreateDepthResources();
    CreateFrameContexts();
    CreateSwapchainSemaphores();

    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr));

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    if (physicalDeviceCount > 0)
    {
        VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data()));
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
        m_queueFamilyIndices.graphicsFamily.value(),
        m_queueFamilyIndices.presentFamily.value());
    spdlog::info(
        "Created swapchain | images={} format={} extent={}x{}",
        m_swapchainImages.size(),
        static_cast<int>(m_swapchainImageFormat),
        m_swapchainExtent.width,
        m_swapchainExtent.height);
}

VulkanInstance::~VulkanInstance()
{
    if (m_device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_device);
    }

    DestroyFrameContexts();
    DestroySwapchainSemaphores();
    DestroyGraphicsPipeline();
    DestroySampleGeometryResources();
    CleanupSwapchain();
    DestroyUploadContext();

    if (m_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

VkInstance VulkanInstance::GetHandle() const { return m_instance; }
VkSurfaceKHR VulkanInstance::GetSurface() const { return m_surface; }
VkPhysicalDevice VulkanInstance::GetPhysicalDevice() const { return m_physicalDevice; }
VkDevice VulkanInstance::GetDevice() const { return m_device; }
VkQueue VulkanInstance::GetGraphicsQueue() const { return m_graphicsQueue; }
VkQueue VulkanInstance::GetPresentQueue() const { return m_presentQueue; }
const QueueFamilyIndices& VulkanInstance::GetQueueFamilyIndices() const { return m_queueFamilyIndices; }
VkSwapchainKHR VulkanInstance::GetSwapchain() const { return m_swapchain; }
VkFormat VulkanInstance::GetSwapchainImageFormat() const { return m_swapchainImageFormat; }
VkExtent2D VulkanInstance::GetSwapchainExtent() const { return m_swapchainExtent; }
const std::vector<VkImage>& VulkanInstance::GetSwapchainImages() const { return m_swapchainImages; }
const std::vector<VkImageView>& VulkanInstance::GetSwapchainImageViews() const { return m_swapchainImageViews; }

void VulkanInstance::DrawFrame(const chimi::renderer::RenderPacket& renderPacket)
{
    SyncGraphicsPipeline(renderPacket);
    SyncMeshResources(renderPacket);
    const PreparedMeshPass preparedMeshPass = BuildPreparedMeshPass(renderPacket);
    FrameContext& frame = GetCurrentFrame();

    VK_CHECK(vkWaitForFences(m_device, 1, &frame.inFlightFence, VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(m_device, 1, &frame.inFlightFence));

    uint32_t imageIndex = 0;
    const VkResult acquireResult = vkAcquireNextImageKHR(
        m_device,
        m_swapchain,
        UINT64_MAX,
        frame.imageAvailableSemaphore,
        VK_NULL_HANDLE,
        &imageIndex);

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        HandleResize();
        return;
    }

    CHIMI_ASSERT(
        acquireResult == VK_SUCCESS || acquireResult == VK_SUBOPTIMAL_KHR,
        "Failed to acquire the next swapchain image");

    VK_CHECK(vkResetCommandPool(m_device, frame.commandPool, 0));
    RecordFrameCommandBuffer(frame, imageIndex, preparedMeshPass);

    const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    const VkSemaphore renderFinishedSemaphore = m_renderFinishedSemaphores[imageIndex];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

    VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, frame.inFlightFence));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &imageIndex;

    const VkResult presentResult = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    CHIMI_ASSERT(
        presentResult == VK_SUCCESS
            || presentResult == VK_SUBOPTIMAL_KHR
            || presentResult == VK_ERROR_OUT_OF_DATE_KHR,
        "Failed to present the swapchain image");

    if (presentResult == VK_SUBOPTIMAL_KHR || presentResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        HandleResize();
    }

    VK_CHECK(vkQueueWaitIdle(m_presentQueue));
    m_currentFrameIndex = (m_currentFrameIndex + 1) % kFramesInFlight;
}

void VulkanInstance::HandleResize()
{
    RecreateSwapchain();
}

std::vector<const char*> VulkanInstance::BuildRequiredExtensions()
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

std::vector<const char*> VulkanInstance::BuildValidationLayers()
{
    if (!ValidationEnabled())
    {
        return {};
    }

    return { "VK_LAYER_KHRONOS_validation" };
}

bool VulkanInstance::ValidationEnabled()
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

FrameContext& VulkanInstance::GetCurrentFrame()
{
    return m_frameContexts[m_currentFrameIndex];
}
}
