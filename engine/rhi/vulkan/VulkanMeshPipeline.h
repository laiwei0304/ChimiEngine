#pragma once

#include "renderer/RenderPacket.h"
#include "rhi/vulkan/VulkanMeshPass.h"

namespace chimi::rhi::vulkan
{
void SyncMeshPassPipeline(
    VulkanMeshPass& meshPass,
    VkDevice device,
    VkFormat colorFormat,
    VkFormat depthFormat,
    const chimi::renderer::MeshPassPipelineConfig& config);
void DestroyMeshPass(VulkanMeshPass& meshPass, VkDevice device);
}
