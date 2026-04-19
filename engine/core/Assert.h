#pragma once

#include <cstdint>
#include <stdexcept>

#include <vulkan/vulkan.h>

namespace chimi::core
{
[[noreturn]] void HandleAssertionFailure(const char* message);
[[noreturn]] void HandleVulkanFailure(VkResult result);
}

#define CHIMI_ASSERT(condition, message)                                             \
    do                                                                               \
    {                                                                                \
        if (!(condition))                                                            \
        {                                                                            \
            ::chimi::core::HandleAssertionFailure((message));                        \
        }                                                                            \
    } while (false)

#define VK_CHECK(expression)                                                         \
    do                                                                               \
    {                                                                                \
        const VkResult chimiVkCheckResult = (expression);                            \
        if (chimiVkCheckResult != VK_SUCCESS)                                        \
        {                                                                            \
            ::chimi::core::HandleVulkanFailure(chimiVkCheckResult);                  \
        }                                                                            \
    } while (false)
