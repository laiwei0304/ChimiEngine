#include <iostream>
#include "CmLog.h"
#include "CmFileUtil.h"
#include "CmGeometryUtil.h"
#include "CmWindow.h"
#include "CmGraphicContext.h"
#include "Graphic/CmVKDevice.h"
#include "Graphic/CmVKQueue.h"
#include "Graphic/CmVKGraphicContext.h"
#include "Graphic/CmVkSwapchain.h"
#include "Graphic/CmVKRenderPass.h"
#include "Graphic/CmVKFrameBuffer.h"
#include "Graphic/CmVKPipeline.h"
#include "Graphic/CmVKCommandBuffer.h"
#include "Graphic/CmVKImage.h"
#include "Graphic/CmVKBuffer.h"

int main(){
    try {
        std::cout << "Hello Chimi Engine." << std::endl;

        chimi::CmLog::Init();
        LOG_T("Hello spdlog: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);
        LOG_D("Hello spdlog: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);
        LOG_I("Hello spdlog: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);
        LOG_W("Hello spdlog: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);
        LOG_E("Hello spdlog: {0}, {1}, {3}", __FUNCTION__, 1, 0.14f, true);

        std::unique_ptr<chimi::CmWindow> window = chimi::CmWindow::Create(800, 600, "00_FirstTriangle");
        std::unique_ptr<chimi::CmGraphicContext> graphicContext = chimi::CmGraphicContext::Create(window.get());
        auto vkContext = dynamic_cast<chimi::CmVKGraphicContext*>(graphicContext.get());
        std::shared_ptr<chimi::CmVKDevice> device = std::make_shared<chimi::CmVKDevice>(vkContext, 1, 1);
        std::shared_ptr<chimi::CmVKSwapchain> swapchain = std::make_shared<chimi::CmVKSwapchain>(vkContext, device.get());
        swapchain->ReCreate();

        std::shared_ptr<chimi::CmVKRenderPass> renderPass = std::make_shared<chimi::CmVKRenderPass>(device.get());

        std::vector<VkImage> swapchainImages = swapchain->GetImages();
        uint32_t swapchainImageSize = swapchainImages.size();
        std::vector<std::shared_ptr<chimi::CmVKFrameBuffer>> framebuffers;
        VkExtent3D imageExtent = { swapchain->GetWidth(), swapchain->GetHeight(), 1 };
        for(int i = 0; i < swapchainImageSize; i++){
            std::vector<std::shared_ptr<chimi::CmVKImage>> images = {
                std::make_shared<chimi::CmVKImage>(device.get(), swapchainImages[i], imageExtent, device->GetSettings().surfaceFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
            };
            framebuffers.push_back(std::make_shared<chimi::CmVKFrameBuffer>(device.get(), renderPass.get(), images, swapchain->GetWidth(), swapchain->GetHeight()));
        }

        const std::string shaderOutputDir = CHIMI_SHADER_OUTPUT_DIR;
        std::shared_ptr<chimi::CmVKPipelineLayout> pipelineLayout = std::make_shared<chimi::CmVKPipelineLayout>(
            device.get(),
            shaderOutputDir + "/00_hello_triangle.vert",
            shaderOutputDir + "/00_hello_triangle.frag");
        std::shared_ptr<chimi::CmVKPipeline> pipeline = std::make_shared<chimi::CmVKPipeline>(device.get(), renderPass.get(), pipelineLayout.get());
        pipeline->SetInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        pipeline->SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        pipeline->Create();

        std::shared_ptr<chimi::CmVKCommandPool> cmdPool = std::make_shared<chimi::CmVKCommandPool>(device.get(), vkContext->GetGraphicQueueFamilyInfo().queueFamilyIndex);
        std::vector<VkCommandBuffer> cmdBuffers = cmdPool->AllocateCommandBuffer(swapchainImages.size());

        chimi::CmVKQueue *graphicQueue = device->GetFirstGraphicQueue();
        const std::vector<VkClearValue> clearValues = {
                { .color = { 0.1f, 0.2f, 0.3f, 1.f }}, { .depthStencil = { 1, 0 } }
        };

        VkFence acquireFence;
        VkFenceCreateInfo fenceInfo = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0

        };
        CALL_VK(vkCreateFence(device->GetHandle(), &fenceInfo, nullptr, &acquireFence));

        while (!window->ShouldClose()){
            window->PollEvents();

            // 1. Acquire the next swapchain image.
            int32_t imageIndex;
            swapchain->AcquireImage(&imageIndex, VK_NULL_HANDLE, acquireFence);

            // 2. Begin the command buffer.
            chimi::CmVKCommandPool::BeginCommandBuffer(cmdBuffers[imageIndex]);

            // 3. Begin the render pass with the matching framebuffer.
            renderPass->Begin(cmdBuffers[imageIndex], framebuffers[imageIndex].get(), clearValues);

            // 4. Bind pipeline, geometry, descriptor sets, etc.
            pipeline->Bind(cmdBuffers[imageIndex]);

            // Viewport and scissor are dynamic states in this pipeline.
            VkViewport viewport = {
                    .x = 0,
                    .y = 0,
                    .width = static_cast<float>(framebuffers[imageIndex]->GetWidth()),
                    .height = static_cast<float>(framebuffers[imageIndex]->GetHeight()),
                    .minDepth = 0.f,
                    .maxDepth = 1.f
            };
            vkCmdSetViewport(cmdBuffers[imageIndex], 0, 1, &viewport);
            VkRect2D scissor = {
                    .offset = { 0, 0 },
                    .extent = { framebuffers[imageIndex]->GetWidth(), framebuffers[imageIndex]->GetHeight() }
            };
            vkCmdSetScissor(cmdBuffers[imageIndex], 0, 1, &scissor);

            // 5. Draw.
            vkCmdDraw(cmdBuffers[imageIndex], 3, 1, 0, 0);

            // 6. End the render pass.
            renderPass->End(cmdBuffers[imageIndex]);
            // 7. End the command buffer.
            chimi::CmVKCommandPool::EndCommandBuffer(cmdBuffers[imageIndex]);

            // 8. Submit the command buffer.
            graphicQueue->Submit({ cmdBuffers[imageIndex] }, {  }, {  });
            graphicQueue->WaitIdle();

            // 9. Present.
            swapchain->Present(imageIndex, {  });

            window->SwapBuffer();
        }

        vkDeviceWaitIdle(device->GetHandle());
        VK_D(Fence, device->GetHandle(), acquireFence);

        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
