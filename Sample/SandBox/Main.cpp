#include "CmEntryPoint.h"
#include "CmFileUtil.h"
#include "Graphic/CmVKRenderPass.h"
#include "Graphic/CmVKCommandBuffer.h"

#include "Render/CmRenderTarget.h"
#include "Render/CmMesh.h"
#include "Render/CmRenderer.h"
#include "Render/CmMaterial.h"

#include "ECS/CmEntity.h"
#include "ECS/System/CmBaseMaterialSystem.h"
#include "ECS/System/CmUnlitMaterialSystem.h"
#include "ECS/Component/CmLookAtCameraComponent.h"

#include "CmEventTesting.h"

class SandBoxApp : public chimi::CmApplication{
protected:
    void OnConfiguration(chimi::AppSettings *appSettings) override {
        appSettings->width = 1360;
        appSettings->height = 768;
        appSettings->title = "SandBox";
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
        mRenderTarget->CmdMaterialSystem<chimi::CmBaseMaterialSystem>();
        mRenderTarget->CmdMaterialSystem<chimi::CmUnlitMaterialSystem>();

        mRenderer = std::make_shared<chimi::CmRenderer>();

        mCmdBuffers = device->GetDefaultCmdPool()->AllocateCommandBuffer(swapchain->GetImages().size());

        // Event
        mEventTesting = std::make_shared<CmEventTesting>();
        mObserver = std::make_shared<chimi::CmEventObserver>();
        mObserver->OnEvent<chimi::CmMouseScrollEvent>([this](const chimi::CmMouseScrollEvent &event){
            chimi::CmEntity *camera = mRenderTarget->GetCamera();
            if(chimi::CmEntity::HasComponent<chimi::CmLookAtCameraComponent>(camera)){
                auto &cameraComp = camera->GetComponent<chimi::CmLookAtCameraComponent>();
                float radius = cameraComp.GetRadius() + event.mYOffset * -0.3f;
                if(radius < 0.1f){
                    radius = 0.1f;
                }
                cameraComp.SetRadius(radius);
            }
        });

        std::vector<chimi::CmVertex> vertices;
        std::vector<uint32_t> indices;
        chimi::CmGeometryUtil::CreateCube(-0.1f, 0.1f, -0.1f, 0.1f, -0.1f, 0.1f, vertices, indices);
        mCubeMesh = std::make_shared<chimi::CmMesh>(vertices, indices);

        mDefaultSampler = std::make_shared<chimi::CmSampler>(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
        chimi::RGBAColor whiteColor = { 255, 255, 255, 255 };
        chimi::RGBAColor blackColor = { 0, 0, 0, 0 };
        chimi::RGBAColor multiColors[4] = {
            255, 255, 255, 255,
            192, 192, 192, 255,
            192, 192, 192, 255,
            255, 255, 255, 255
        };
        mWhiteTexture = std::make_shared<chimi::CmTexture>(1, 1, &whiteColor);
        mBlackTexture = std::make_shared<chimi::CmTexture>(1, 1, &blackColor);
        mMultiPixelTexture = std::make_shared<chimi::CmTexture>(2, 2, multiColors);
        mFileTexture = std::make_shared<chimi::CmTexture>(CHIMI_RES_TEXTURE_DIR"R-C.jpeg");
    }

    void OnSceneInit(chimi::CmScene *scene) override {
        chimi::CmEntity *camera = scene->CreateEntity("Editor Camera");
        camera->AddComponent<chimi::CmLookAtCameraComponent>();
        mRenderTarget->SetCamera(camera);

        float x = -2.f;
        for(int i = 0; i < mSmallCubeSize.x; i++, x+=0.5f){
            float y = -2.f;
            for(int j = 0; j < mSmallCubeSize.y; j++, y+=0.5f){
                float z = -2.f;
                for(int k = 0; k < mSmallCubeSize.z; k++, z+=0.5f){
                    chimi::CmEntity *cube = scene->CreateEntity("Cube");
                    auto &transComp = cube->GetComponent<chimi::CmTransformComponent>();
                    transComp.position = { x, y, z };
                    mSmallCubes.push_back(cube);
                }
            }
        }
    }

    void OnSceneDestroy(chimi::CmScene *scene) override {

    }

    void OnUpdate(float deltaTime) override {
        uint64_t frameIndex = GetFrameIndex();

        chimi::CmTexture *textures[] = { mWhiteTexture.get(), mBlackTexture.get(), mMultiPixelTexture.get(), mFileTexture.get() };
        if(frameIndex % 2 == 0 && mUnlitMaterials.size() < mSmallCubes.size()){
            auto material = chimi::CmMaterialFactory::GetInstance()->CreateMaterial<chimi::CmUnlitMaterial>();
            material->SetBaseColor0(glm::linearRand(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f)));
            material->SetBaseColor1(glm::linearRand(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f)));
            material->SetTextureView(chimi::UNLIT_MAT_BASE_COLOR_0, textures[glm::linearRand(0, (int)ARRAY_SIZE(textures) - 1)], mDefaultSampler.get());
            material->SetTextureView(chimi::UNLIT_MAT_BASE_COLOR_1, textures[glm::linearRand(0, (int)ARRAY_SIZE(textures) - 1)], mDefaultSampler.get());
            material->UpdateTextureViewEnable(chimi::UNLIT_MAT_BASE_COLOR_0, glm::linearRand(0, 1));
            material->UpdateTextureViewEnable(chimi::UNLIT_MAT_BASE_COLOR_1, glm::linearRand(0, 1));
            material->SetMixValue(glm::linearRand(0.1f, 0.8f));

            uint32_t cubeIndex = mUnlitMaterials.size();
            if(!chimi::CmEntity::HasComponent<chimi::CmUnlitMaterialComponent>(mSmallCubes[cubeIndex])){
                mSmallCubes[cubeIndex]->AddComponent<chimi::CmUnlitMaterialComponent>();
            }
            auto &materialComp = mSmallCubes[cubeIndex]->GetComponent<chimi::CmUnlitMaterialComponent>();
            materialComp.CmdMesh(mCubeMesh.get(), material);

            mUnlitMaterials.push_back(material);
            LOG_D("Unlit Material Count: {0}", mUnlitMaterials.size());
        }

        if(frameIndex % 20 == 0 && !mUnlitMaterials.empty()){
            mUnlitMaterials[0]->SetMixValue(glm::linearRand(0.f, 1.f));
            mUnlitMaterials[0]->SetTextureView(chimi::UNLIT_MAT_BASE_COLOR_0, textures[glm::linearRand(0, (int)ARRAY_SIZE(textures) - 1)], mDefaultSampler.get());
        }

        chimi::CmEntity *camera = mRenderTarget->GetCamera();
        if(chimi::CmEntity::HasComponent<chimi::CmLookAtCameraComponent>(camera)){
            if(!mWindow->IsMouseDown()){
                bFirstMouseDrag = true;
                return;
            }

            glm::vec2 mousePos;
            mWindow->GetMousePos(mousePos);
            glm::vec2 mousePosDelta = { mLastMousePos.x - mousePos.x, mousePos.y - mLastMousePos.y };
            mLastMousePos = mousePos;

            if(abs(mousePosDelta.x) > 0.1f || abs(mousePosDelta.y) > 0.1f){
                if(bFirstMouseDrag){
                    bFirstMouseDrag = false;
                } else {
                    auto &transComp = camera->GetComponent<chimi::CmTransformComponent>();
                    float yaw = transComp.rotation.x;
                    float pitch = transComp.rotation.y;

                    yaw += mousePosDelta.x * mMouseSensitivity;
                    pitch += mousePosDelta.y * mMouseSensitivity;

                    if(pitch > 89.f){
                        pitch = 89.f;
                    }
                    if(pitch < -89.f){
                        pitch = -89.f;
                    }
                    transComp.rotation.x = yaw;
                    transComp.rotation.y = pitch;
                }
            }
        }
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
        mDefaultSampler.reset();
        mWhiteTexture.reset();
        mBlackTexture.reset();
        mMultiPixelTexture.reset();
        mFileTexture.reset();
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
    std::shared_ptr<chimi::CmTexture> mWhiteTexture;
    std::shared_ptr<chimi::CmTexture> mBlackTexture;
    std::shared_ptr<chimi::CmTexture> mMultiPixelTexture;
    std::shared_ptr<chimi::CmTexture> mFileTexture;
    std::shared_ptr<chimi::CmSampler> mDefaultSampler;
    glm::ivec3 mSmallCubeSize{ 10, 10, 10 }; // x, y, z
    std::vector<chimi::CmEntity*> mSmallCubes;
    std::vector<chimi::CmUnlitMaterial*> mUnlitMaterials;

    std::shared_ptr<CmEventTesting> mEventTesting;
    std::shared_ptr<chimi::CmEventObserver> mObserver;

    bool bFirstMouseDrag = true;
    glm::vec2 mLastMousePos;
    float mMouseSensitivity = 0.25f;
};

chimi::CmApplication *CreateApplicationEntryPoint(){
    return new SandBoxApp();
}
