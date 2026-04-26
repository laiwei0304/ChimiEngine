#include "ECS/System/CmBaseMaterialSystem.h"
#include "CmFileUtil.h"
#include "CmGeometryUtil.h"
#include "CmApplication.h"

#include "Render/CmRenderContext.h"
#include "Render/CmRenderTarget.h"

#include "Graphic/CmVKPipeline.h"
#include "Graphic/CmVKFrameBuffer.h"

#include "ECS/CmEntity.h"
#include "ECS/Component/CmLookAtCameraComponent.h"

namespace chimi{
    void CmBaseMaterialSystem::OnInit(CmVKRenderPass *renderPass) {
        chimi::CmVKDevice *device = GetDevice();

        chimi::ShaderLayout shaderLayout = {
            .pushConstants = {
                {
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .offset = 0,
                    .size = sizeof(PushConstants)
                }
            }
        };
        mPipelineLayout = std::make_shared<CmVKPipelineLayout>(device,
                                                               CHIMI_SHADER_OUTPUT_DIR"/01_hello_buffer.vert",
                                                               CHIMI_SHADER_OUTPUT_DIR"/01_hello_buffer.frag",
                                                               shaderLayout);
        std::vector<VkVertexInputBindingDescription> vertexBindings = {
            {
                .binding = 0,
                .stride = sizeof(CmVertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            }
        };
        std::vector<VkVertexInputAttributeDescription> vertexAttrs = {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(CmVertex, position)
            },
            {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(CmVertex, texcoord0)
            },
            {
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(CmVertex, normal)
            }
        };
        mPipeline = std::make_shared<CmVKPipeline>(device, renderPass, mPipelineLayout.get());
        mPipeline->SetVertexInputState(vertexBindings, vertexAttrs);
        mPipeline->SetInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)->EnableDepthTest();
        mPipeline->SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        mPipeline->SetMultisampleState(VK_SAMPLE_COUNT_4_BIT, VK_FALSE);
        mPipeline->Create();
    }

    void CmBaseMaterialSystem::OnRender(VkCommandBuffer cmdBuffer, CmRenderTarget *renderTarget, uint32_t frameIndex) {
        (void)frameIndex;

        CmScene *scene = GetScene();
        if(!scene){
            return;
        }

        entt::registry &reg = scene->GetEcsRegistry();
        auto view = reg.view<CmTransformComponent, CmBaseMaterialComponent>();
        if(view.begin() == view.end()){
            return;
        }
        // bind pipeline
        mPipeline->Bind(cmdBuffer);
        // setup global params
        CmVKFrameBuffer *frameBuffer = renderTarget->GetFrameBuffer();
        VkViewport viewport = {
                .x = 0,
                .y = 0,
                .width = static_cast<float>(frameBuffer->GetWidth()),
                .height = static_cast<float>(frameBuffer->GetHeight()),
                .minDepth = 0.f,
                .maxDepth = 1.f
        };
        vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
        VkRect2D scissor = {
                .offset = { 0, 0 },
                .extent = { frameBuffer->GetWidth(), frameBuffer->GetHeight() }
        };
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

        glm::mat4 projMat = GetProjMat(renderTarget);
        glm::mat4 viewMat = GetViewMat(renderTarget);

        // setup custom params
        view.each([this, &cmdBuffer, &projMat, &viewMat](const auto &e, const CmTransformComponent &transComp, const CmBaseMaterialComponent &materialComp){
            auto meshMaterials = materialComp.GetMeshMaterials();
            for (const auto &entry: meshMaterials){
                CmBaseMaterial *material = entry.first;
                if(!material){
                    LOG_W("TODO: default material or error material ?");
                    continue;
                }
                PushConstants pushConstants {
                        .matrix = projMat * viewMat * transComp.GetTransform(),
                        .colorType = static_cast<uint32_t>(material->colorType)
                };
                vkCmdPushConstants(cmdBuffer, mPipelineLayout->GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstants), &pushConstants);

                // mesh list draw
                for (const auto &meshIndex: entry.second){
                    CmMesh *mesh = materialComp.GetMesh(meshIndex);
                    if(mesh){
                        mesh->Draw(cmdBuffer);
                    }
                }
            }
        });
    }

    void CmBaseMaterialSystem::OnDestroy() {
        mPipeline.reset();
        mPipelineLayout.reset();
    }
}
