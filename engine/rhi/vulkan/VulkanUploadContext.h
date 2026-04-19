#pragma once

#include "rhi/vulkan/VulkanContext.h"

#include <functional>

#include <vulkan/vulkan.h>

namespace chimi::rhi::vulkan
{
struct AllocatedBuffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct UploadContext
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
};

struct VulkanUploadContext
{
    UploadContext uploadContext{};
};

using UploadFindMemoryTypeFn = std::function<uint32_t(uint32_t, VkMemoryPropertyFlags)>;
using RecordImmediateCommandsFn = std::function<void(VkCommandBuffer)>;

void CreateUploadContext(VulkanUploadContext& uploadContext, const VulkanContext& context);
void DestroyUploadContext(VulkanUploadContext& uploadContext, VkDevice device);
AllocatedBuffer CreateBuffer(
    VkDevice device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    const UploadFindMemoryTypeFn& findMemoryType);
void DestroyBuffer(VkDevice device, AllocatedBuffer& buffer);
void ImmediateSubmit(
    const VulkanUploadContext& uploadContext,
    VkDevice device,
    VkQueue graphicsQueue,
    const RecordImmediateCommandsFn& recordCommands);
void UploadBufferWithStaging(
    const VulkanUploadContext& uploadContext,
    VkDevice device,
    VkQueue graphicsQueue,
    const void* data,
    VkDeviceSize size,
    AllocatedBuffer& destinationBuffer,
    VkBufferUsageFlags usage,
    const UploadFindMemoryTypeFn& findMemoryType);
}
