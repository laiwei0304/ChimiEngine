#pragma once

#include "renderer/MeshData.h"
#include "rhi/vulkan/VulkanContext.h"
#include "rhi/vulkan/VulkanUploadContext.h"

#include <functional>

namespace chimi::rhi::vulkan
{
struct MeshBuffer
{
    AllocatedBuffer vertexBuffer{};
    AllocatedBuffer indexBuffer{};
    uint32_t indexCount = 0;
};

using MeshBufferFindMemoryTypeFn = std::function<uint32_t(uint32_t, VkMemoryPropertyFlags)>;

MeshBuffer CreateMeshBuffer(
    const VulkanContext& context,
    const VulkanUploadContext& uploadContext,
    const chimi::renderer::CpuMeshData& meshData,
    const MeshBufferFindMemoryTypeFn& findMemoryType);
void DrawMeshBuffer(VkCommandBuffer commandBuffer, const MeshBuffer& meshBuffer);
void DestroyMeshBuffer(VkDevice device, MeshBuffer& meshBuffer);
}
