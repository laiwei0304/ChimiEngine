#include "rhi/vulkan/VulkanFrameContext.h"

#include "core/Assert.h"

#include <volk.h>

namespace chimi::rhi::vulkan
{
void CreateFrameContexts(
    std::vector<FrameContext>& frameContexts,
    VkDevice device,
    uint32_t queueFamilyIndex,
    uint32_t framesInFlight)
{
    frameContexts.resize(framesInFlight);

    for (FrameContext& frame : frameContexts)
    {
        VkCommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolCreateInfo.queueFamilyIndex = queueFamilyIndex;
        VK_CHECK(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &frame.commandPool));

        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = frame.commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;
        VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &frame.commandBuffer));

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.imageAvailableSemaphore));

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &frame.inFlightFence));
    }
}

void DestroyFrameContexts(std::vector<FrameContext>& frameContexts, VkDevice device)
{
    for (FrameContext& frame : frameContexts)
    {
        if (frame.inFlightFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(device, frame.inFlightFence, nullptr);
            frame.inFlightFence = VK_NULL_HANDLE;
        }

        if (frame.imageAvailableSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, frame.imageAvailableSemaphore, nullptr);
            frame.imageAvailableSemaphore = VK_NULL_HANDLE;
        }

        if (frame.commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device, frame.commandPool, nullptr);
            frame.commandPool = VK_NULL_HANDLE;
            frame.commandBuffer = VK_NULL_HANDLE;
        }
    }

    frameContexts.clear();
}

FrameContext& GetCurrentFrame(std::vector<FrameContext>& frameContexts, uint32_t currentFrameIndex)
{
    return frameContexts[currentFrameIndex];
}

void WaitForFrame(FrameContext& frame, VkDevice device)
{
    VK_CHECK(vkWaitForFences(device, 1, &frame.inFlightFence, VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(device, 1, &frame.inFlightFence));
}

AcquireFrameResult AcquireFrameImage(VkDevice device, VkSwapchainKHR swapchain, const FrameContext& frame)
{
    AcquireFrameResult acquireResult{};
    acquireResult.result = vkAcquireNextImageKHR(
        device,
        swapchain,
        UINT64_MAX,
        frame.imageAvailableSemaphore,
        VK_NULL_HANDLE,
        &acquireResult.imageIndex);
    return acquireResult;
}

void ResetFrameCommandPool(VkDevice device, const FrameContext& frame)
{
    VK_CHECK(vkResetCommandPool(device, frame.commandPool, 0));
}

void SubmitFrame(
    VkQueue graphicsQueue,
    FrameContext& frame,
    VkSemaphore renderFinishedSemaphore)
{
    const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, frame.inFlightFence));
}

VkResult PresentFrame(
    VkQueue presentQueue,
    VkSwapchainKHR swapchain,
    uint32_t imageIndex,
    VkSemaphore renderFinishedSemaphore)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(presentQueue, &presentInfo);
}

uint32_t AdvanceFrameIndex(uint32_t currentFrameIndex, uint32_t framesInFlight)
{
    return (currentFrameIndex + 1) % framesInFlight;
}
}
