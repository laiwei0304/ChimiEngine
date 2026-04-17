#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "spdlog/spdlog.h"

#define CHIMI_ASSERT(condition, message)                                             \
    do                                                                               \
    {                                                                                \
        if (!(condition))                                                            \
        {                                                                            \
            spdlog::critical("Assertion failed: {}", (message));                    \
            throw std::runtime_error(message);                                       \
        }                                                                            \
    } while (false)

#define VK_CHECK(expression)                                                         \
    do                                                                               \
    {                                                                                \
        const VkResult chimiVkCheckResult = (expression);                            \
        if (chimiVkCheckResult != VK_SUCCESS)                                        \
        {                                                                            \
            spdlog::critical("Vulkan call failed with VkResult={}",                 \
                static_cast<int>(chimiVkCheckResult));                               \
            throw std::runtime_error("Vulkan call failed");                         \
        }                                                                            \
    } while (false)
