#include "rhi/vulkan/VulkanMeshPass.h"

#include "core/Assert.h"

#include <stdexcept>
#include <volk.h>

namespace chimi::rhi::vulkan
{
namespace
{
struct MeshDrawPushConstants
{
    glm::mat4 objectToClip{ 1.0f };
    glm::vec4 lightDirectionAndIntensity{ -0.6f, -1.0f, -0.35f, 1.0f };
    glm::vec4 lightColorAndAmbient{ 1.0f, 0.98f, 0.92f, 0.18f };
};

void TransitionSwapchainImage(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else
    {
        throw std::runtime_error("Unsupported swapchain image layout transition");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);
}

void TransitionDepthImage(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    else if (
        oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
        && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::runtime_error("Unsupported depth image layout transition");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);
}
}

PreparedMeshPass BuildPreparedMeshPass(
    const chimi::renderer::RenderPacket& renderPacket,
    const VulkanMeshCache& meshCache)
{
    const chimi::renderer::MeshPassPacket& meshPass = renderPacket.mainMeshPass;
    PreparedMeshPass preparedMeshPass{};
    preparedMeshPass.clearColor[0] = meshPass.clearColor.r;
    preparedMeshPass.clearColor[1] = meshPass.clearColor.g;
    preparedMeshPass.clearColor[2] = meshPass.clearColor.b;
    preparedMeshPass.clearColor[3] = meshPass.clearColor.a;
    preparedMeshPass.draws.reserve(meshPass.draws.size());

    for (const chimi::renderer::MeshDrawPacket& draw : meshPass.draws)
    {
        const MeshGpuResource* meshGpuResource = FindMeshGpuResource(meshCache, draw.meshId);
        CHIMI_ASSERT(meshGpuResource != nullptr, "Mesh draw must reference a synchronized GPU mesh resource");
        preparedMeshPass.draws.push_back(PreparedMeshDraw{
            .meshBuffer = &meshGpuResource->meshBuffer,
            .objectToClip = draw.objectToClip,
            .lightDirectionAndIntensity = draw.lightDirectionAndIntensity,
            .lightColorAndAmbient = draw.lightColorAndAmbient
        });
    }

    return preparedMeshPass;
}

void RecordPreparedMeshPassCommands(
    const MeshPassCommandContext& context,
    const PreparedMeshPass& preparedMeshPass,
    const DrawMeshBufferFn& drawMeshBuffer)
{
    CHIMI_ASSERT(context.swapchain != nullptr, "Mesh pass command context must contain a swapchain");
    CHIMI_ASSERT(context.meshPass != nullptr, "Mesh pass command context must contain mesh pass state");
    CHIMI_ASSERT(context.imageIndex < context.swapchain->images.size(), "Swapchain image index is out of range");

    VulkanSwapchain& swapchain = *context.swapchain;

    TransitionSwapchainImage(
        context.commandBuffer,
        swapchain.images[context.imageIndex],
        swapchain.imageLayouts[context.imageIndex],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    TransitionDepthImage(
        context.commandBuffer,
        swapchain.depthImage.image,
        swapchain.depthImageLayout,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkClearValue clearValue{};
    clearValue.color = {
        {
            preparedMeshPass.clearColor[0],
            preparedMeshPass.clearColor[1],
            preparedMeshPass.clearColor[2],
            preparedMeshPass.clearColor[3]
        }
    };
    VkClearValue depthClearValue{};
    depthClearValue.depthStencil = { 1.0f, 0 };

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = swapchain.imageViews[context.imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = clearValue;

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = swapchain.depthImage.imageView;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue = depthClearValue;

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = { 0, 0 };
    renderingInfo.renderArea.extent = swapchain.extent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;

    vkCmdBeginRendering(context.commandBuffer, &renderingInfo);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain.extent.width);
    viewport.height = static_cast<float>(swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain.extent;

    vkCmdSetViewport(context.commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(context.commandBuffer, 0, 1, &scissor);
    vkCmdBindPipeline(
        context.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        context.meshPass->graphicsPipeline);

    for (const PreparedMeshDraw& draw : preparedMeshPass.draws)
    {
        const MeshDrawPushConstants pushConstants{
            .objectToClip = draw.objectToClip,
            .lightDirectionAndIntensity = draw.lightDirectionAndIntensity,
            .lightColorAndAmbient = draw.lightColorAndAmbient
        };
        vkCmdPushConstants(
            context.commandBuffer,
            context.meshPass->pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(MeshDrawPushConstants),
            &pushConstants);
        CHIMI_ASSERT(draw.meshBuffer != nullptr, "Prepared mesh draw must reference a valid mesh buffer");
        drawMeshBuffer(context.commandBuffer, *draw.meshBuffer);
    }

    vkCmdEndRendering(context.commandBuffer);

    TransitionSwapchainImage(
        context.commandBuffer,
        swapchain.images[context.imageIndex],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    swapchain.imageLayouts[context.imageIndex] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    swapchain.depthImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
}
}
