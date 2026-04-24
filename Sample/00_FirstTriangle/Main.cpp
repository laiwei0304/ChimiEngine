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

        std::cout << "Step: create window" << std::endl;
        std::unique_ptr<chimi::CmWindow> window = chimi::CmWindow::Create(800, 600, "00_FirstTriangle");
        std::cout << "Step: create graphic context" << std::endl;
        std::unique_ptr<chimi::CmGraphicContext> graphicContext = chimi::CmGraphicContext::Create(window.get());
        auto vkContext = dynamic_cast<chimi::CmVKGraphicContext*>(graphicContext.get());
        std::cout << "Step: create device" << std::endl;
        std::shared_ptr<chimi::CmVKDevice> device = std::make_shared<chimi::CmVKDevice>(vkContext, 1, 1);
        std::cout << "Step: create swapchain" << std::endl;
        std::shared_ptr<chimi::CmVKSwapchain> swapchain = std::make_shared<chimi::CmVKSwapchain>(vkContext, device.get());
        swapchain->ReCreate();

        std::cout << "Step: create render pass" << std::endl;
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
        std::cout << "Step: create pipeline layout" << std::endl;
        std::shared_ptr<chimi::CmVKPipelineLayout> pipelineLayout = std::make_shared<chimi::CmVKPipelineLayout>(
            device.get(),
            shaderOutputDir + "/00_hello_triangle.vert",
            shaderOutputDir + "/00_hello_triangle.frag");
        std::cout << "Step: create pipeline" << std::endl;
        std::shared_ptr<chimi::CmVKPipeline> pipeline = std::make_shared<chimi::CmVKPipeline>(device.get(), renderPass.get(), pipelineLayout.get());
        pipeline->SetInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        pipeline->SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        pipeline->Create();

        std::cout << "Step: create command pool" << std::endl;
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

        std::cout << "Step: enter main loop" << std::endl;
        bool firstFrame = true;
        while (!window->ShouldClose()){
            window->PollEvents();

            int32_t imageIndex;
            if (firstFrame) {
                std::cout << "Frame Step: acquire" << std::endl;
            }
            swapchain->AcquireImage(&imageIndex, VK_NULL_HANDLE, acquireFence);

            if (firstFrame) {
                std::cout << "Frame Step: record" << std::endl;
            }
            chimi::CmVKCommandPool::BeginCommandBuffer(cmdBuffers[imageIndex]);
            renderPass->Begin(cmdBuffers[imageIndex], framebuffers[imageIndex].get(), clearValues);
            pipeline->Bind(cmdBuffers[imageIndex]);

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

            vkCmdDraw(cmdBuffers[imageIndex], 3, 1, 0, 0);

            renderPass->End(cmdBuffers[imageIndex]);
            chimi::CmVKCommandPool::EndCommandBuffer(cmdBuffers[imageIndex]);
            if (firstFrame) {
                std::cout << "Frame Step: submit" << std::endl;
            }
            graphicQueue->Submit({ cmdBuffers[imageIndex] }, {  }, {  });
            graphicQueue->WaitIdle();

            if (firstFrame) {
                std::cout << "Frame Step: present" << std::endl;
            }
            swapchain->Present(imageIndex, {  });
            window->SwapBuffer();
            if (firstFrame) {
                std::cout << "Frame Step: complete" << std::endl;
                firstFrame = false;
            }
        }

        vkDeviceWaitIdle(device->GetHandle());
        VK_D(Fence, device->GetHandle(), acquireFence);

        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
