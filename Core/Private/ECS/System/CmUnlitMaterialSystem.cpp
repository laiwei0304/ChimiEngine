#include "ECS/System/CmUnlitMaterialSystem.h"

#include "CmFileUtil.h"
#include "CmApplication.h"
#include "Graphic/CmVKPipeline.h"
#include "Graphic/CmVKDescriptorSet.h"
#include "Graphic/CmVKImageView.h"
#include "Graphic/CmVKFrameBuffer.h"

#include "Render/CmRenderTarget.h"

#include "ECS/Component/CmTransformComponent.h"

namespace chimi{
    void CmUnlitMaterialSystem::OnInit(CmVKRenderPass *renderPass) {
        CmVKDevice *device = GetDevice();

        //Frame Ubo
        {
             const std::vector<VkDescriptorSetLayoutBinding> bindings = {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                }
            };
            mFrameUboDescSetLayout = std::make_shared<CmVKDescriptorSetLayout>(device, bindings);
        }

        // Material Params
        {
            const std::vector<VkDescriptorSetLayoutBinding> bindings = {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
                }
            };
            mMaterialParamDescSetLayout = std::make_shared<CmVKDescriptorSetLayout>(device, bindings);
        }

        // Material Resource
        {
            const std::vector<VkDescriptorSetLayoutBinding> bindings = {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
                },
                {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
                }
            };
            mMaterialResourceDescSetLayout = std::make_shared<CmVKDescriptorSetLayout>(device, bindings);
        }

        VkPushConstantRange modelPC = {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(ModelPC)
        };

        ShaderLayout shaderLayout = {
            .descriptorSetLayouts = { mFrameUboDescSetLayout->GetHandle(), mMaterialParamDescSetLayout->GetHandle(), mMaterialResourceDescSetLayout->GetHandle() },
            .pushConstants = { modelPC }
        };
        mPipelineLayout = std::make_shared<CmVKPipelineLayout>(device,
                                                               CHIMI_RES_SHADER_DIR"03_unlit_material.vert",
                                                               CHIMI_RES_SHADER_DIR"03_unlit_material.frag",
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
        mPipeline->EnableDepthTest();
        mPipeline->SetDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
        mPipeline->SetMultisampleState(VK_SAMPLE_COUNT_4_BIT, VK_FALSE);
        mPipeline->Create();

        std::vector<VkDescriptorPoolSize> poolSizes = {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1
            }
        };
        mDescriptorPool = std::make_shared<CmVKDescriptorPool>(device, 1, poolSizes);
        mFrameUboDescSet = mDescriptorPool->AllocateDescriptorSet(mFrameUboDescSetLayout.get(), 1)[0];
        mFrameUboBuffer = std::make_shared<chimi::CmVKBuffer>(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(FrameUbo), nullptr, true);

        ReCreateMaterialDescPool(NUM_MATERIAL_BATCH);
    }

    void CmUnlitMaterialSystem::OnRender(VkCommandBuffer cmdBuffer, CmRenderTarget *renderTarget) {
        CmScene *scene = GetScene();
        if(!scene){
            return;
        }
        entt::registry &reg = scene->GetEcsRegistry();

        auto view = reg.view<CmTransformComponent, CmUnlitMaterialComponent>();
        if(view.begin() == view.end()){
            return;
        }

        mPipeline->Bind(cmdBuffer);
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

        UpdateFrameUboDescSet(renderTarget);

        bool bShouldForceUpdateMaterial = false;
        uint32_t materialCount = CmMaterialFactory::GetInstance()->GetMaterialSize<CmUnlitMaterial>();
        if(materialCount > mLastDescriptorSetCount){
            ReCreateMaterialDescPool(materialCount);
            bShouldForceUpdateMaterial = true;
        }

        std::vector<bool> updateFlags(materialCount);
        view.each([this, &updateFlags, &bShouldForceUpdateMaterial, &cmdBuffer](CmTransformComponent &transComp, CmUnlitMaterialComponent &materialComp){
            for (const auto &entry: materialComp.GetMeshMaterials()){
                CmUnlitMaterial *material = entry.first;
                if(!material || material->GetIndex() < 0){
                    LOG_W("TODO: default material or error material ?");
                    continue;
                }

                uint32_t materialIndex = material->GetIndex();
                VkDescriptorSet paramsDescSet = mMaterialDescSets[materialIndex];
                VkDescriptorSet resourceDescSet = mMaterialResourceDescSets[materialIndex];

                if(!updateFlags[materialIndex]){
                    if(material->ShouldFlushParams() || bShouldForceUpdateMaterial){
                        //LOG_T("Update material params : {0}", materialIndex);
                        UpdateMaterialParamsDescSet(paramsDescSet, material);
                        material->FinishFlushParams();
                    }
                    if(material->ShouldFlushResource() || bShouldForceUpdateMaterial){
                        //LOG_T("Update material resource : {0}", materialIndex);
                        UpdateMaterialResourceDescSet(resourceDescSet, material);
                        material->FinishFlushResource();
                    }
                    updateFlags[materialIndex] = true;
                }

                VkDescriptorSet descriptorSets[] = { mFrameUboDescSet, paramsDescSet, resourceDescSet };
                vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout->GetHandle(),
                                        0, ARRAY_SIZE(descriptorSets), descriptorSets, 0, nullptr);

                ModelPC pc = { transComp.GetTransform() };
                vkCmdPushConstants(cmdBuffer, mPipelineLayout->GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);

                for (const auto &meshIndex: entry.second){
                    materialComp.GetMesh(meshIndex)->Draw(cmdBuffer);
                }
            }
        });
    }

    void CmUnlitMaterialSystem::OnDestroy() {

    }

    void CmUnlitMaterialSystem::ReCreateMaterialDescPool(uint32_t materialCount) {
        CmVKDevice *device = GetDevice();

        uint32_t newDescriptorSetCount = mLastDescriptorSetCount;
        if(mLastDescriptorSetCount == 0){
            newDescriptorSetCount = NUM_MATERIAL_BATCH;
        }

        while (newDescriptorSetCount < materialCount) {
            newDescriptorSetCount *= 2;
        }

        if(newDescriptorSetCount > NUM_MATERIAL_BATCH_MAX){
            LOG_E("Descriptor Set max count is : {0}, but request : {1}", NUM_MATERIAL_BATCH_MAX, newDescriptorSetCount);
            return;
        }

        LOG_W("{0}: {1} -> {2} S.", __FUNCTION__, mLastDescriptorSetCount, newDescriptorSetCount);

        // Destroy old
        mMaterialDescSets.clear();
        mMaterialResourceDescSets.clear();
        if(mMaterialDescriptorPool){
            mMaterialDescriptorPool.reset();
        }

        std::vector<VkDescriptorPoolSize> poolSizes = {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = newDescriptorSetCount
            },
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = newDescriptorSetCount * 2               // because has color_tex0 and color_tex1
            }
        };
        mMaterialDescriptorPool = std::make_shared<chimi::CmVKDescriptorPool>(device, newDescriptorSetCount * 2, poolSizes);  //because has params and resource desc. set

        mMaterialDescSets = mMaterialDescriptorPool->AllocateDescriptorSet(mMaterialParamDescSetLayout.get(), newDescriptorSetCount);
        mMaterialResourceDescSets = mMaterialDescriptorPool->AllocateDescriptorSet(mMaterialResourceDescSetLayout.get(), newDescriptorSetCount);
        assert(mMaterialDescSets.size() == newDescriptorSetCount && "Failed to AllocateDescriptorSet");
        assert(mMaterialResourceDescSets.size() == newDescriptorSetCount && "Failed to AllocateDescriptorSet");

        uint32_t diffCount = newDescriptorSetCount - mLastDescriptorSetCount;
        for(int i = 0; i < diffCount; i++){
            mMaterialBuffers.push_back(std::make_shared<CmVKBuffer>(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UnlitMaterialUbo), nullptr, true));
        }
        LOG_W("{0}: {1} -> {2} E.", __FUNCTION__, mLastDescriptorSetCount, newDescriptorSetCount);
        mLastDescriptorSetCount = newDescriptorSetCount;
    }

    void CmUnlitMaterialSystem::UpdateFrameUboDescSet(CmRenderTarget *renderTarget) {
        CmApplication *app = GetApp();
        CmVKDevice *device = GetDevice();

        CmVKFrameBuffer *frameBuffer = renderTarget->GetFrameBuffer();
        glm::ivec2 resolution = { frameBuffer->GetWidth(), frameBuffer->GetHeight() };

        FrameUbo frameUbo = {
            .projMat = GetProjMat(renderTarget),
            .viewMat = GetViewMat(renderTarget),
            .resolution = resolution,
            .frameId = static_cast<uint32_t>(app->GetFrameIndex()),
            .time = app->GetStartTimeSecond()
        };

        mFrameUboBuffer->WriteData(&frameUbo);
        VkDescriptorBufferInfo bufferInfo = DescriptorSetWriter::BuildBufferInfo(mFrameUboBuffer->GetHandle(), 0, sizeof(frameUbo));
        VkWriteDescriptorSet bufferWrite = DescriptorSetWriter::WriteBuffer(mFrameUboDescSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo);
        DescriptorSetWriter::UpdateDescriptorSets(device->GetHandle(), { bufferWrite });
    }

    void CmUnlitMaterialSystem::UpdateMaterialParamsDescSet(VkDescriptorSet descSet, CmUnlitMaterial *material) {
        CmVKDevice *device = GetDevice();

        CmVKBuffer *materialBuffer = mMaterialBuffers[material->GetIndex()].get();

        UnlitMaterialUbo params = material->GetParams();

        const TextureView *texture0 = material->GetTextureView(UNLIT_MAT_BASE_COLOR_0);
        if(texture0){
            CmMaterial::UpdateTextureParams(texture0, &params.textureParam0);
        }

        const TextureView *texture1 = material->GetTextureView(UNLIT_MAT_BASE_COLOR_1);
        if(texture1){
            CmMaterial::UpdateTextureParams(texture1, &params.textureParam1);
        }

        materialBuffer->WriteData(&params);
        VkDescriptorBufferInfo bufferInfo = DescriptorSetWriter::BuildBufferInfo(materialBuffer->GetHandle(), 0, sizeof(params));
        VkWriteDescriptorSet bufferWrite = DescriptorSetWriter::WriteBuffer(descSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo);
        DescriptorSetWriter::UpdateDescriptorSets(device->GetHandle(), { bufferWrite });
    }

    void CmUnlitMaterialSystem::UpdateMaterialResourceDescSet(VkDescriptorSet descSet, CmUnlitMaterial *material) {
        CmVKDevice *device = GetDevice();

        const TextureView *texture0 = material->GetTextureView(UNLIT_MAT_BASE_COLOR_0);
        const TextureView *texture1 = material->GetTextureView(UNLIT_MAT_BASE_COLOR_1);

        VkDescriptorImageInfo textureInfo0 = DescriptorSetWriter::BuildImageInfo(texture0->sampler->GetHandle(), texture0->texture->GetImageView()->GetHandle());
        VkDescriptorImageInfo textureInfo1 = DescriptorSetWriter::BuildImageInfo(texture1->sampler->GetHandle(), texture1->texture->GetImageView()->GetHandle());

        VkWriteDescriptorSet textureWrite0 = DescriptorSetWriter::WriteImage(descSet, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureInfo0);
        VkWriteDescriptorSet textureWrite1 = DescriptorSetWriter::WriteImage(descSet, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureInfo1);

        DescriptorSetWriter::UpdateDescriptorSets(device->GetHandle(), { textureWrite0, textureWrite1 });
    }
}