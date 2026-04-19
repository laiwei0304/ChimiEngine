#include "rhi/vulkan/VulkanFrameRecording.h"

#include "core/Assert.h"

#include <volk.h>

namespace chimi::rhi::vulkan
{
void RecordFrameCommandBuffer(
    FrameContext& frame,
    uint32_t imageIndex,
    VulkanSwapchain& swapchain,
    const VulkanMeshPass& meshPass,
    const PreparedMeshPass& preparedMeshPass)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(frame.commandBuffer, &beginInfo));

    RecordPreparedMeshPassCommands(
        MeshPassCommandContext{
            .commandBuffer = frame.commandBuffer,
            .imageIndex = imageIndex,
            .swapchain = &swapchain,
            .meshPass = &meshPass
        },
        preparedMeshPass,
        chimi::rhi::vulkan::DrawMeshBuffer);

    VK_CHECK(vkEndCommandBuffer(frame.commandBuffer));
}
}
