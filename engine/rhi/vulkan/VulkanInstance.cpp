#include "rhi/vulkan/VulkanInstance.h"

#include "core/Assert.h"

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

VulkanInstance::VulkanInstance()
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
}

VulkanInstance::~VulkanInstance()
{
    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

VkInstance VulkanInstance::GetHandle() const
{
    return m_instance;
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
}
