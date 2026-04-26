#include "ECS/System/CmUnlitMaterialSystem.h"

#include "CmFileUtil.h"
#include "CmApplication.h"
#include "Graphic/CmVKPipeline.h"
#include "Graphic/CmVKDescriptorSet.h"
#include "Graphic/CmVKImageView.h"
#include "Graphic/CmVKFrameBuffer.h"

#include "Render/CmRenderer.h"
#include "Render/CmRenderTarget.h"

#include "ECS/Component/CmTransformComponent.h"

namespace chimi{
    static bool HasTextureBinding(const TextureView *textureView) {
        return textureView && textureView->texture && textureView->sampler;
    }

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
                                                               CHIMI_SHADER_OUTPUT_DIR"/03_unlit_material.vert",
                                                               CHIMI_SHADER_OUTPUT_DIR"/03_unlit_material.frag",
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

        InitFrameUboDescriptors();

        mMaterialDescriptorPools.resize(RENDERER_NUM_BUFFER);
        mMaterialDescriptorSetCounts.resize(RENDERER_NUM_BUFFER);
        mMaterialDescSets.resize(RENDERER_NUM_BUFFER);
        mMaterialResourceDescSets.resize(RENDERER_NUM_BUFFER);
        mMaterialBuffers.resize(RENDERER_NUM_BUFFER);
        mMaterialParamVersions.resize(RENDERER_NUM_BUFFER);
        mMaterialResourceVersions.resize(RENDERER_NUM_BUFFER);
        mMaterialResourceValid.resize(RENDERER_NUM_BUFFER);
    }

    void CmUnlitMaterialSystem::OnRender(VkCommandBuffer cmdBuffer, CmRenderTarget *renderTarget, uint32_t frameIndex) {
        CmScene *scene = GetScene();
        if(!scene){
            return;
        }
        frameIndex = frameIndex % RENDERER_NUM_BUFFER;
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

        UpdateFrameUbo(frameIndex, renderTarget);

        uint32_t materialCount = CmMaterialFactory::GetInstance()->GetMaterialSize<CmUnlitMaterial>();
        EnsureMaterialCapacity(frameIndex, materialCount);

        std::vector<bool> updateFlags(materialCount);
        view.each([this, frameIndex, &updateFlags, &cmdBuffer](CmTransformComponent &transComp, CmUnlitMaterialComponent &materialComp){
            for (const auto &entry: materialComp.GetMeshMaterials()){
                CmUnlitMaterial *material = entry.first;
                if(!material || material->GetIndex() < 0){
                    LOG_W("TODO: default material or error material ?");
                    continue;
                }

                uint32_t materialIndex = material->GetIndex();
                VkDescriptorSet paramsDescSet = mMaterialDescSets[frameIndex][materialIndex];
                VkDescriptorSet resourceDescSet = mMaterialResourceDescSets[frameIndex][materialIndex];

                if(!updateFlags[materialIndex]){
                    if(mMaterialParamVersions[frameIndex][materialIndex] != material->GetParamsVersion()){
                        UpdateMaterialParamsDescSet(frameIndex, materialIndex, material);
                    }
                    if(AreMaterialParamsSynced(materialIndex, material)){
                        material->FinishFlushParams();
                    }
                    if(mMaterialResourceVersions[frameIndex][materialIndex] != material->GetResourceVersion()){
                        if(!UpdateMaterialResourceDescSet(frameIndex, materialIndex, material)){
                            continue;
                        }
                    }
                    if(!mMaterialResourceValid[frameIndex][materialIndex]){
                        continue;
                    }
                    if(AreMaterialResourcesSynced(materialIndex, material)){
                        material->FinishFlushResource();
                    }
                    updateFlags[materialIndex] = true;
                }

                VkDescriptorSet descriptorSets[] = { mFrameUboDescSets[frameIndex], paramsDescSet, resourceDescSet };
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

    void CmUnlitMaterialSystem::InitFrameUboDescriptors() {
        CmVKDevice *device = GetDevice();

        std::vector<VkDescriptorPoolSize> poolSizes = {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = RENDERER_NUM_BUFFER
            }
        };
        mDescriptorPool = std::make_shared<CmVKDescriptorPool>(device, RENDERER_NUM_BUFFER, poolSizes);
        mFrameUboDescSets = mDescriptorPool->AllocateDescriptorSet(mFrameUboDescSetLayout.get(), RENDERER_NUM_BUFFER);
        mFrameUboBuffers.resize(RENDERER_NUM_BUFFER);

        std::vector<VkWriteDescriptorSet> writes;
        std::vector<VkDescriptorBufferInfo> bufferInfos(RENDERER_NUM_BUFFER);
        for(uint32_t frameIndex = 0; frameIndex < RENDERER_NUM_BUFFER; frameIndex++){
            mFrameUboBuffers[frameIndex] = std::make_shared<CmVKBuffer>(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(FrameUbo), nullptr, true);
            bufferInfos[frameIndex] = DescriptorSetWriter::BuildBufferInfo(mFrameUboBuffers[frameIndex]->GetHandle(), 0, sizeof(FrameUbo));
            writes.push_back(DescriptorSetWriter::WriteBuffer(mFrameUboDescSets[frameIndex], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfos[frameIndex]));
        }
        DescriptorSetWriter::UpdateDescriptorSets(device->GetHandle(), writes);
    }

    void CmUnlitMaterialSystem::EnsureMaterialCapacity(uint32_t frameIndex, uint32_t materialCount) {
        CmVKDevice *device = GetDevice();

        uint32_t newDescriptorSetCount = mMaterialDescriptorSetCounts[frameIndex];
        if(newDescriptorSetCount == 0){
            newDescriptorSetCount = NUM_MATERIAL_BATCH;
        }
        while (newDescriptorSetCount < materialCount) {
            newDescriptorSetCount *= 2;
        }

        if(newDescriptorSetCount == mMaterialDescriptorSetCounts[frameIndex]){
            return;
        }
        if(newDescriptorSetCount > NUM_MATERIAL_BATCH_MAX){
            LOG_E("Descriptor Set max count is : {0}, but request : {1}", NUM_MATERIAL_BATCH_MAX, newDescriptorSetCount);
            return;
        }

        LOG_D("{0}: frame {1}, {2} -> {3} S.", __FUNCTION__, frameIndex, mMaterialDescriptorSetCounts[frameIndex], newDescriptorSetCount);

        mMaterialDescSets[frameIndex].clear();
        mMaterialResourceDescSets[frameIndex].clear();
        mMaterialDescriptorPools[frameIndex].reset();

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
        mMaterialDescriptorPools[frameIndex] = std::make_shared<CmVKDescriptorPool>(device, newDescriptorSetCount * 2, poolSizes);  //because has params and resource desc. set

        mMaterialDescSets[frameIndex] = mMaterialDescriptorPools[frameIndex]->AllocateDescriptorSet(mMaterialParamDescSetLayout.get(), newDescriptorSetCount);
        mMaterialResourceDescSets[frameIndex] = mMaterialDescriptorPools[frameIndex]->AllocateDescriptorSet(mMaterialResourceDescSetLayout.get(), newDescriptorSetCount);
        assert(mMaterialDescSets[frameIndex].size() == newDescriptorSetCount && "Failed to AllocateDescriptorSet");
        assert(mMaterialResourceDescSets[frameIndex].size() == newDescriptorSetCount && "Failed to AllocateDescriptorSet");

        mMaterialBuffers[frameIndex].clear();
        mMaterialBuffers[frameIndex].reserve(newDescriptorSetCount);
        for(uint32_t i = 0; i < newDescriptorSetCount; i++){
            mMaterialBuffers[frameIndex].push_back(std::make_shared<CmVKBuffer>(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UnlitMaterialUbo), nullptr, true));
        }

        mMaterialParamVersions[frameIndex].assign(newDescriptorSetCount, 0);
        mMaterialResourceVersions[frameIndex].assign(newDescriptorSetCount, 0);
        mMaterialResourceValid[frameIndex].assign(newDescriptorSetCount, false);

        LOG_D("{0}: frame {1}, {2} -> {3} E.", __FUNCTION__, frameIndex, mMaterialDescriptorSetCounts[frameIndex], newDescriptorSetCount);
        mMaterialDescriptorSetCounts[frameIndex] = newDescriptorSetCount;
    }

    void CmUnlitMaterialSystem::UpdateFrameUbo(uint32_t frameIndex, CmRenderTarget *renderTarget) {
        CmApplication *app = GetApp();

        CmVKFrameBuffer *frameBuffer = renderTarget->GetFrameBuffer();
        glm::ivec2 resolution = { frameBuffer->GetWidth(), frameBuffer->GetHeight() };

        FrameUbo frameUbo = {
            .projMat = GetProjMat(renderTarget),
            .viewMat = GetViewMat(renderTarget),
            .resolution = resolution,
            .frameId = static_cast<uint32_t>(app->GetFrameIndex()),
            .time = app->GetStartTimeSecond()
        };

        mFrameUboBuffers[frameIndex]->WriteData(&frameUbo);
    }

    void CmUnlitMaterialSystem::UpdateMaterialParamsDescSet(uint32_t frameIndex, uint32_t materialIndex, CmUnlitMaterial *material) {
        CmVKDevice *device = GetDevice();

        CmVKBuffer *materialBuffer = mMaterialBuffers[frameIndex][materialIndex].get();

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
        VkDescriptorSet descSet = mMaterialDescSets[frameIndex][materialIndex];
        VkWriteDescriptorSet bufferWrite = DescriptorSetWriter::WriteBuffer(descSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo);
        DescriptorSetWriter::UpdateDescriptorSets(device->GetHandle(), { bufferWrite });
        mMaterialParamVersions[frameIndex][materialIndex] = material->GetParamsVersion();
    }

    bool CmUnlitMaterialSystem::UpdateMaterialResourceDescSet(uint32_t frameIndex, uint32_t materialIndex, CmUnlitMaterial *material) {
        CmVKDevice *device = GetDevice();

        const TextureView *texture0 = material->GetTextureView(UNLIT_MAT_BASE_COLOR_0);
        const TextureView *texture1 = material->GetTextureView(UNLIT_MAT_BASE_COLOR_1);
        if(!HasTextureBinding(texture0) || !HasTextureBinding(texture1)){
            mMaterialResourceVersions[frameIndex][materialIndex] = material->GetResourceVersion();
            mMaterialResourceValid[frameIndex][materialIndex] = false;
            return false;
        }

        VkDescriptorImageInfo textureInfo0 = DescriptorSetWriter::BuildImageInfo(texture0->sampler->GetHandle(), texture0->texture->GetImageView()->GetHandle());
        VkDescriptorImageInfo textureInfo1 = DescriptorSetWriter::BuildImageInfo(texture1->sampler->GetHandle(), texture1->texture->GetImageView()->GetHandle());

        VkDescriptorSet descSet = mMaterialResourceDescSets[frameIndex][materialIndex];
        VkWriteDescriptorSet textureWrite0 = DescriptorSetWriter::WriteImage(descSet, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureInfo0);
        VkWriteDescriptorSet textureWrite1 = DescriptorSetWriter::WriteImage(descSet, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureInfo1);

        DescriptorSetWriter::UpdateDescriptorSets(device->GetHandle(), { textureWrite0, textureWrite1 });
        mMaterialResourceVersions[frameIndex][materialIndex] = material->GetResourceVersion();
        mMaterialResourceValid[frameIndex][materialIndex] = true;
        return true;
    }

    bool CmUnlitMaterialSystem::AreMaterialParamsSynced(uint32_t materialIndex, const CmUnlitMaterial *material) const {
        for(const auto &versions: mMaterialParamVersions){
            if(versions.size() <= materialIndex || versions[materialIndex] != material->GetParamsVersion()){
                return false;
            }
        }
        return true;
    }

    bool CmUnlitMaterialSystem::AreMaterialResourcesSynced(uint32_t materialIndex, const CmUnlitMaterial *material) const {
        for(const auto &versions: mMaterialResourceVersions){
            if(versions.size() <= materialIndex || versions[materialIndex] != material->GetResourceVersion()){
                return false;
            }
        }
        return true;
    }
}
