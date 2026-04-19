#pragma once

#include "rhi/vulkan/VulkanFrameContext.h"
#include "rhi/vulkan/VulkanMeshPass.h"

namespace chimi::rhi::vulkan
{
void RecordFrameCommandBuffer(
    FrameContext& frame,
    uint32_t imageIndex,
    VulkanSwapchain& swapchain,
    const VulkanMeshPass& meshPass,
    const PreparedMeshPass& preparedMeshPass);
}
