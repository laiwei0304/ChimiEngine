#include "core/Assert.h"

#include "spdlog/spdlog.h"

namespace chimi::core
{
[[noreturn]] void HandleAssertionFailure(const char* message)
{
    spdlog::critical("Assertion failed: {}", message);
    throw std::runtime_error(message);
}

[[noreturn]] void HandleVulkanFailure(VkResult result)
{
    spdlog::critical("Vulkan call failed with VkResult={}", static_cast<int>(result));
    throw std::runtime_error("Vulkan call failed");
}
}
