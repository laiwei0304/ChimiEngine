#include "rhi/vulkan/VulkanUploadContext.h"

#include "core/Assert.h"

#include <cstring>
#include <volk.h>

namespace chimi::rhi::vulkan
{
void CreateUploadContext(VulkanUploadContext& uploadContext, const VulkanContext& context)
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolCreateInfo.queueFamilyIndex = context.queueFamilyIndices.graphicsFamily.value();
    VK_CHECK(vkCreateCommandPool(context.device, &poolCreateInfo, nullptr, &uploadContext.uploadContext.commandPool));

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VK_CHECK(vkCreateFence(context.device, &fenceCreateInfo, nullptr, &uploadContext.uploadContext.fence));
}

void DestroyUploadContext(VulkanUploadContext& uploadContext, VkDevice device)
{
    if (uploadContext.uploadContext.fence != VK_NULL_HANDLE)
    {
        vkDestroyFence(device, uploadContext.uploadContext.fence, nullptr);
        uploadContext.uploadContext.fence = VK_NULL_HANDLE;
    }

    if (uploadContext.uploadContext.commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, uploadContext.uploadContext.commandPool, nullptr);
        uploadContext.uploadContext.commandPool = VK_NULL_HANDLE;
    }
}

AllocatedBuffer CreateBuffer(
    VkDevice device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    const UploadFindMemoryTypeFn& findMemoryType)
{
    AllocatedBuffer buffer{};

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer.buffer));

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(device, buffer.buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);
    VK_CHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &buffer.memory));
    VK_CHECK(vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0));

    return buffer;
}

void DestroyBuffer(VkDevice device, AllocatedBuffer& buffer)
{
    if (buffer.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, buffer.buffer, nullptr);
        buffer.buffer = VK_NULL_HANDLE;
    }

    if (buffer.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, buffer.memory, nullptr);
        buffer.memory = VK_NULL_HANDLE;
    }
}

void ImmediateSubmit(
    const VulkanUploadContext& uploadContext,
    VkDevice device,
    VkQueue graphicsQueue,
    const RecordImmediateCommandsFn& recordCommands)
{
    CHIMI_ASSERT(uploadContext.uploadContext.commandPool != VK_NULL_HANDLE, "Upload command pool must exist");
    CHIMI_ASSERT(uploadContext.uploadContext.fence != VK_NULL_HANDLE, "Upload fence must exist");

    VK_CHECK(vkResetFences(device, 1, &uploadContext.uploadContext.fence));
    VK_CHECK(vkResetCommandPool(device, uploadContext.uploadContext.commandPool, 0));

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = uploadContext.uploadContext.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    recordCommands(commandBuffer);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, uploadContext.uploadContext.fence));
    VK_CHECK(vkWaitForFences(device, 1, &uploadContext.uploadContext.fence, VK_TRUE, UINT64_MAX));

    vkFreeCommandBuffers(device, uploadContext.uploadContext.commandPool, 1, &commandBuffer);
}

void UploadBufferWithStaging(
    const VulkanUploadContext& uploadContext,
    VkDevice device,
    VkQueue graphicsQueue,
    const void* data,
    VkDeviceSize size,
    AllocatedBuffer& destinationBuffer,
    VkBufferUsageFlags usage,
    const UploadFindMemoryTypeFn& findMemoryType)
{
    CHIMI_ASSERT(data != nullptr, "Upload data must not be null");
    CHIMI_ASSERT(size > 0, "Upload size must be greater than zero");

    AllocatedBuffer stagingBuffer = CreateBuffer(
        device,
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        findMemoryType);

    void* mappedData = nullptr;
    VK_CHECK(vkMapMemory(device, stagingBuffer.memory, 0, size, 0, &mappedData));
    std::memcpy(mappedData, data, static_cast<size_t>(size));
    vkUnmapMemory(device, stagingBuffer.memory);

    destinationBuffer = CreateBuffer(
        device,
        size,
        usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        findMemoryType);

    ImmediateSubmit(uploadContext, device, graphicsQueue, [&](VkCommandBuffer commandBuffer)
    {
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer.buffer, destinationBuffer.buffer, 1, &copyRegion);
    });

    DestroyBuffer(device, stagingBuffer);
}
}
