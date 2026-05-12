#include "CmEntryPoint.h"
#include "CmFileUtil.h"
#include "Render/CmRenderTarget.h"
#include "Render/CmMesh.h"
#include "Render/CmRenderer.h"
#include "Graphic/CmVKRenderPass.h"
#include "Graphic/CmVKCommandBuffer.h"

#include "ECS/CmEntity.h"
#include "ECS/System/CmBaseMaterialSystem.h"
#include "ECS/Component/CmLookAtCameraComponent.h"

class SandBoxApp : public chimi::CmApplication{
protected:
    void OnConfiguration(chimi::AppSettings *appSettings) override {
        appSettings->width = 1360;
        appSettings->height = 768;
        appSettings->title = "04_ECS_Entity";
    }

    void OnInit() override {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();
        chimi::CmVKSwapchain *swapchain = renderCxt->GetSwapchain();

        std::vector<chimi::Attachment> attachments = {
            {
                .format = swapchain->GetSurfaceInfo().surfaceFormat.format,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            },
            {
                .format = device->GetSettings().depthFormat,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            }
        };
        std::vector<chimi::RenderSubPass> subpasses = {
            {
                .colorAttachments = { 0 },
                .depthStencilAttachments = { 1 },
                .sampleCount = VK_SAMPLE_COUNT_4_BIT
            }
        };
        mRenderPass = std::make_shared<chimi::CmVKRenderPass>(device, attachments, subpasses);

        mRenderTarget = std::make_shared<chimi::CmRenderTarget>(mRenderPass.get());
        mRenderTarget->SetColorClearValue({0.1f, 0.2f, 0.3f, 1.f});
        mRenderTarget->SetDepthStencilClearValue({ 1, 0 });
        mRenderTarget->AddMaterialSystem<chimi::CmBaseMaterialSystem>();

        mRenderer = std::make_shared<chimi::CmRenderer>();

        mCmdBuffers = device->GetDefaultCmdPool()->AllocateCommandBuffer(swapchain->GetImages().size());

        std::vector<chimi::CmVertex> vertices;
        std::vector<uint32_t> indices;
        chimi::CmGeometryUtil::CreateCube(-0.3f, 0.3f, -0.3f, 0.3f, -0.3f, 0.3f, vertices, indices);
        mCubeMesh = std::make_shared<chimi::CmMesh>(vertices, indices);
    }

    void OnSceneInit(chimi::CmScene *scene) override {
        chimi::CmEntity *camera = scene->CreateEntity("Editor Camera");
        auto &cameraComp = camera->AddComponent<chimi::CmLookAtCameraComponent>();
        cameraComp.SetRadius(2.f);
        mRenderTarget->SetCamera(camera);

        auto baseMat0 = chimi::CmMaterialFactory::GetInstance()->CreateMaterial<chimi::CmBaseMaterial>();
        baseMat0->colorType = chimi::COLOR_TYPE_NORMAL;
        auto baseMat1 = chimi::CmMaterialFactory::GetInstance()->CreateMaterial<chimi::CmBaseMaterial>();
        baseMat1->colorType = chimi::COLOR_TYPE_TEXCOORD;

        {
            chimi::CmEntity *cube = scene->CreateEntity("Cube 0");
            auto &materialComp = cube->AddComponent<chimi::CmBaseMaterialComponent>();
            materialComp.AddMesh(mCubeMesh.get(), baseMat1);
            auto &transComp = cube->GetComponent<chimi::CmTransformComponent>();
            transComp.scale = { 1.f, 1.f, 1.f };
            transComp.position = { 0.f, 0.f, 0.0f };
            transComp.rotation = { 17.f, 30.f, 0.f };
        }
        {
            chimi::CmEntity *cube = scene->CreateEntity("Cube 1");
            auto &materialComp = cube->AddComponent<chimi::CmBaseMaterialComponent>();
            materialComp.AddMesh(mCubeMesh.get(), baseMat0);
            auto &transComp = cube->GetComponent<chimi::CmTransformComponent>();
            transComp.scale = { 0.5f, 0.5f, 0.5f };
            transComp.position = { -1.f, 0.f, 0.0f };
            transComp.rotation = { 17.f, 30.f, 0.f };
        }
        {
            chimi::CmEntity *cube = scene->CreateEntity("Cube 2");
            auto &materialComp = cube->AddComponent<chimi::CmBaseMaterialComponent>();
            materialComp.AddMesh(mCubeMesh.get(), baseMat1);
            auto &transComp = cube->GetComponent<chimi::CmTransformComponent>();
            transComp.scale = { 0.5f, 0.5f, 0.5f };
            transComp.position = { 1.f, 0.f, 0.0f };
            transComp.rotation = { 17.f, 30.f, 0.f };
        }
    }

    void OnSceneDestroy(chimi::CmScene *scene) override {

    }

    void OnRender() override {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKSwapchain *swapchain = renderCxt->GetSwapchain();

        int32_t imageIndex;
        if(mRenderer->Begin(&imageIndex)){
            mRenderTarget->SetExtent({ swapchain->GetWidth(), swapchain->GetHeight() });
        }
        uint32_t frameIndex = mRenderer->GetCurrentBufferIndex();

        VkCommandBuffer cmdBuffer = mCmdBuffers[imageIndex];
        chimi::CmVKCommandPool::BeginCommandBuffer(cmdBuffer);

        mRenderTarget->Begin(cmdBuffer);
        mRenderTarget->RenderMaterialSystems(cmdBuffer, frameIndex);
        mRenderTarget->End(cmdBuffer);

        chimi::CmVKCommandPool::EndCommandBuffer(cmdBuffer);
        if(mRenderer->End(imageIndex, { cmdBuffer })){
            mRenderTarget->SetExtent({ swapchain->GetWidth(), swapchain->GetHeight() });
        }
    }

    void OnDestroy() override {
        chimi::CmRenderContext *renderCxt = chimi::CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();
        vkDeviceWaitIdle(device->GetHandle());
        mCubeMesh.reset();
        mCmdBuffers.clear();
        mRenderTarget.reset();
        mRenderPass.reset();
        mRenderer.reset();
    }
private:
    std::shared_ptr<chimi::CmVKRenderPass> mRenderPass;
    std::shared_ptr<chimi::CmRenderTarget> mRenderTarget;
    std::shared_ptr<chimi::CmRenderer> mRenderer;

    std::vector<VkCommandBuffer> mCmdBuffers;
    std::shared_ptr<chimi::CmMesh> mCubeMesh;
};

chimi::CmApplication *CreateApplicationEntryPoint(){
    return new SandBoxApp();
}