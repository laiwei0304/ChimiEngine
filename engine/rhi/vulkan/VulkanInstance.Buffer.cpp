#include "rhi/vulkan/VulkanInstance.h"

#include "core/Assert.h"

#include <cstring>

#include <volk.h>

namespace chimi::rhi::vulkan
{
void VulkanInstance::CreateUploadContext()
{
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolCreateInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily.value();
    VK_CHECK(vkCreateCommandPool(m_device, &poolCreateInfo, nullptr, &m_uploadContext.commandPool));

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_uploadContext.fence));
}

void VulkanInstance::DestroyUploadContext()
{
    if (m_uploadContext.fence != VK_NULL_HANDLE)
    {
        vkDestroyFence(m_device, m_uploadContext.fence, nullptr);
        m_uploadContext.fence = VK_NULL_HANDLE;
    }

    if (m_uploadContext.commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_device, m_uploadContext.commandPool, nullptr);
        m_uploadContext.commandPool = VK_NULL_HANDLE;
    }
}

AllocatedBuffer VulkanInstance::CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties)
{
    AllocatedBuffer buffer{};

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &buffer.buffer));

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(m_device, buffer.buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);
    VK_CHECK(vkAllocateMemory(m_device, &allocateInfo, nullptr, &buffer.memory));
    VK_CHECK(vkBindBufferMemory(m_device, buffer.buffer, buffer.memory, 0));

    return buffer;
}

void VulkanInstance::DestroyBuffer(AllocatedBuffer& buffer)
{
    if (buffer.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_device, buffer.buffer, nullptr);
        buffer.buffer = VK_NULL_HANDLE;
    }

    if (buffer.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_device, buffer.memory, nullptr);
        buffer.memory = VK_NULL_HANDLE;
    }
}

void VulkanInstance::UploadBufferWithStaging(
    const void* data,
    VkDeviceSize size,
    AllocatedBuffer& destinationBuffer,
    VkBufferUsageFlags usage)
{
    CHIMI_ASSERT(data != nullptr, "Upload data must not be null");
    CHIMI_ASSERT(size > 0, "Upload size must be greater than zero");

    AllocatedBuffer stagingBuffer = CreateBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* mappedData = nullptr;
    VK_CHECK(vkMapMemory(m_device, stagingBuffer.memory, 0, size, 0, &mappedData));
    std::memcpy(mappedData, data, static_cast<size_t>(size));
    vkUnmapMemory(m_device, stagingBuffer.memory);

    destinationBuffer = CreateBuffer(
        size,
        usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CopyBuffer(stagingBuffer.buffer, destinationBuffer.buffer, size);
    DestroyBuffer(stagingBuffer);
}

void VulkanInstance::CopyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size)
{
    ImmediateSubmit([&](VkCommandBuffer commandBuffer)
    {
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);
    });
}

void VulkanInstance::ImmediateSubmit(const std::function<void(VkCommandBuffer)>& recordCommands)
{
    CHIMI_ASSERT(m_uploadContext.commandPool != VK_NULL_HANDLE, "Upload command pool must exist");
    CHIMI_ASSERT(m_uploadContext.fence != VK_NULL_HANDLE, "Upload fence must exist");

    VK_CHECK(vkResetFences(m_device, 1, &m_uploadContext.fence));
    VK_CHECK(vkResetCommandPool(m_device, m_uploadContext.commandPool, 0));

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_uploadContext.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VK_CHECK(vkAllocateCommandBuffers(m_device, &allocateInfo, &commandBuffer));

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
    VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_uploadContext.fence));
    VK_CHECK(vkWaitForFences(m_device, 1, &m_uploadContext.fence, VK_TRUE, UINT64_MAX));

    vkFreeCommandBuffers(m_device, m_uploadContext.commandPool, 1, &commandBuffer);
}
}
