#include "CmEditorApp.h"

#include "Render/CmRenderContext.h"
#include "Graphic/CmVKCommandBuffer.h"
#include "Graphic/CmVKDevice.h"
#include "Graphic/CmVKSwapchain.h"
#include "Graphic/CmVKImageView.h"
#include "ECS/CmScene.h"
#include "ECS/CmEntity.h"
#include "ECS/Component/CmLookAtCameraComponent.h"
#include "ECS/Component/CmTransformComponent.h"
#include "ECS/System/CmUnlitMaterialSystem.h"
#include "CmGeometryUtil.h"
#include "CmFileUtil.h"

#include <algorithm>

namespace chimi{
    void CmEditorApp::OnConfiguration(AppSettings *appSettings) {
        appSettings->width = 1360;
        appSettings->height = 768;
        appSettings->title = "Chimi Editor";
        appSettings->decorated = false;
    }

    void CmEditorApp::OnInit() {
        CmRenderContext *renderContext = CmApplication::GetAppContext()->renderCxt;
        CmVKDevice *device = renderContext->GetDevice();
        CmVKSwapchain *swapchain = renderContext->GetSwapchain();

        std::vector<Attachment> attachments = {
            {
                .format = swapchain->GetSurfaceInfo().surfaceFormat.format,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            }
        };
        std::vector<RenderSubPass> subpasses = {
            {
                .colorAttachments = { 0 },
                .sampleCount = VK_SAMPLE_COUNT_1_BIT
            }
        };
        mUiRenderPass = std::make_shared<CmVKRenderPass>(device, attachments, subpasses);
        mUiRenderTarget = std::make_shared<CmRenderTarget>(mUiRenderPass.get());
        mUiRenderTarget->SetColorClearValue({ 0.94f, 0.95f, 0.96f, 1.f });

        std::vector<Attachment> viewportAttachments = {
            {
                .format = swapchain->GetSurfaceInfo().surfaceFormat.format,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
            },
            {
                .format = device->GetSettings().depthFormat,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            }
        };
        std::vector<RenderSubPass> viewportSubpasses = {
            {
                .colorAttachments = { 0 },
                .depthStencilAttachments = { 1 },
                .sampleCount = VK_SAMPLE_COUNT_4_BIT
            }
        };
        mViewportRenderPass = std::make_shared<CmVKRenderPass>(device, viewportAttachments, viewportSubpasses);
        const std::vector<Attachment> &renderPassAttachments = mViewportRenderPass->GetAttachments();
        for(uint32_t i = 0; i < renderPassAttachments.size(); i++){
            if(!IsDepthStencilFormat(renderPassAttachments[i].format) &&
               renderPassAttachments[i].samples == VK_SAMPLE_COUNT_1_BIT &&
               renderPassAttachments[i].finalLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
                mViewportResolveAttachmentIndex = i;
                break;
            }
        }
        mViewportTargetWidth = swapchain->GetWidth();
        mViewportTargetHeight = swapchain->GetHeight();
        mViewportRenderTarget = std::make_shared<CmRenderTarget>(mViewportRenderPass.get(), RENDERER_NUM_BUFFER, VkExtent2D{ mViewportTargetWidth, mViewportTargetHeight });
        mViewportRenderTarget->SetColorClearValue({ 0.20f, 0.24f, 0.28f, 1.f });
        mViewportRenderTarget->SetDepthStencilClearValue({ 1.f, 0 });
        mViewportRenderTarget->AddMaterialSystem<CmUnlitMaterialSystem>();

        mRenderer = std::make_shared<CmRenderer>();
        mCmdBuffers = device->GetDefaultCmdPool()->AllocateCommandBuffer(swapchain->GetImages().size());
        mTargetWidth = swapchain->GetWidth();
        mTargetHeight = swapchain->GetHeight();
        mTargetImageCount = static_cast<uint32_t>(swapchain->GetImages().size());

        mEditorContext = std::make_shared<CmEditorContext>();
        mEditorContext->SetApp(this);
        mEditorContext->SetViewportSize(swapchain->GetWidth(), swapchain->GetHeight());

        mEditorLayer = std::make_shared<CmEditorLayer>();
        mEditorLayer->Init(mEditorContext.get());

        mImGuiLayer = std::make_shared<CmImGuiLayer>();
        mImGuiLayer->Init(mWindow.get(), renderContext, mUiRenderPass.get());

        mEventObserver = std::make_shared<CmEventObserver>();
        mEventObserver->OnEvent<CmMouseScrollEvent>([this](const CmMouseScrollEvent &event){
            if(mEditorContext && mEditorContext->IsViewportHovered()){
                ZoomEditorCamera(event.mYOffset);
            }
        });

        std::vector<CmVertex> vertices;
        std::vector<uint32_t> indices;
        CmGeometryUtil::CreateCube(-0.5f, 0.5f, -0.5f, 0.5f, -0.5f, 0.5f, vertices, indices);
        mCubeMesh = std::make_shared<CmMesh>(vertices, indices);
        mDefaultSampler = std::make_shared<CmSampler>(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
        mCheckerboardTexture = std::make_shared<CmTexture>(CHIMI_RES_TEXTURE_DIR"Checkerboard.png");
        mPhotoTexture = std::make_shared<CmTexture>(CHIMI_RES_TEXTURE_DIR"R-C.jpeg");
    }

    void CmEditorApp::OnSceneInit(CmScene *scene) {
        if(mEditorContext){
            mEditorContext->SetScene(scene);
            CmEntity *camera = scene->CreateEntity("Editor Camera");
            camera->AddComponent<CmLookAtCameraComponent>();
            auto &cameraTransform = camera->GetComponent<CmTransformComponent>();
            cameraTransform.rotation = { 28.f, 18.f, 0.f };
            camera->GetComponent<CmLookAtCameraComponent>().SetRadius(5.f);
            mViewportRenderTarget->SetCamera(camera);
            mEditorContext->SetSelectedEntity(camera);

            CmEntity *cube0 = scene->CreateEntity("Blue Cube");
            auto &cube0Transform = cube0->GetComponent<CmTransformComponent>();
            cube0Transform.position = { -0.75f, 0.f, 0.f };
            cube0->AddComponent<CmUnlitMaterialComponent>();
            CmUnlitMaterial *blueMat = CmMaterialFactory::GetInstance()->CreateMaterial<CmUnlitMaterial>();
            blueMat->SetBaseColor0({ 0.15f, 0.42f, 0.85f });
            blueMat->SetBaseColor1({ 0.08f, 0.18f, 0.35f });
            blueMat->SetMixValue(0.15f);
            blueMat->SetTextureView(UNLIT_MAT_BASE_COLOR_0, mCheckerboardTexture.get(), mDefaultSampler.get());
            blueMat->SetTextureView(UNLIT_MAT_BASE_COLOR_1, mCheckerboardTexture.get(), mDefaultSampler.get());
            cube0->GetComponent<CmUnlitMaterialComponent>().AddMesh(mCubeMesh.get(), blueMat);

            CmEntity *cube1 = scene->CreateEntity("Gold Cube");
            auto &cube1Transform = cube1->GetComponent<CmTransformComponent>();
            cube1Transform.position = { 0.75f, 0.f, 0.f };
            cube1->AddComponent<CmUnlitMaterialComponent>();
            CmUnlitMaterial *goldMat = CmMaterialFactory::GetInstance()->CreateMaterial<CmUnlitMaterial>();
            goldMat->SetBaseColor0({ 0.92f, 0.62f, 0.18f });
            goldMat->SetBaseColor1({ 0.45f, 0.24f, 0.08f });
            goldMat->SetMixValue(0.2f);
            goldMat->SetTextureView(UNLIT_MAT_BASE_COLOR_0, mPhotoTexture.get(), mDefaultSampler.get());
            goldMat->SetTextureView(UNLIT_MAT_BASE_COLOR_1, mPhotoTexture.get(), mDefaultSampler.get());
            cube1->GetComponent<CmUnlitMaterialComponent>().AddMesh(mCubeMesh.get(), goldMat);

            mViewportMaterials = { blueMat, goldMat };
        }
    }

    void CmEditorApp::OnSceneDestroy(CmScene *scene) {
        if(mEditorContext && mEditorContext->GetScene() == scene){
            mEditorContext->SetScene(nullptr);
        }
    }

    void CmEditorApp::OnUpdate(float deltaTime) {
        mDeltaTime = deltaTime;
        UpdateEditorCamera(deltaTime);
    }

    void CmEditorApp::OnRender() {
        int32_t imageIndex;
        if(mRenderer->Begin(&imageIndex)){
            UpdateSwapchainTarget();
        }
        uint32_t frameIndex = mRenderer->GetCurrentBufferIndex();

        UpdateSwapchainTarget();
        UpdateViewportTarget();

        VkCommandBuffer cmdBuffer = mCmdBuffers[imageIndex];
        CmVKCommandPool::BeginCommandBuffer(cmdBuffer);

        mViewportRenderTarget->Begin(cmdBuffer);
        mViewportRenderTarget->RenderMaterialSystems(cmdBuffer, frameIndex);
        mViewportRenderTarget->End(cmdBuffer);
        UpdateViewportTexture();

        mImGuiLayer->BeginFrame();
        mEditorLayer->OnImGuiRender();

        mUiRenderTarget->Begin(cmdBuffer);
        mImGuiLayer->EndFrame(cmdBuffer);
        mUiRenderTarget->End(cmdBuffer);

        CmVKCommandPool::EndCommandBuffer(cmdBuffer);
        if(mRenderer->End(imageIndex, { cmdBuffer })){
            UpdateSwapchainTarget();
        }
    }

    void CmEditorApp::OnDestroy() {
        CmRenderContext *renderContext = CmApplication::GetAppContext()->renderCxt;
        CmVKDevice *device = renderContext->GetDevice();
        vkDeviceWaitIdle(device->GetHandle());

        mImGuiLayer.reset();
        mEditorLayer.reset();
        mEditorContext.reset();
        mEventObserver.reset();
        mViewportTextureIds.clear();
        mViewportImageViews.clear();
        mViewportMaterials.clear();
        mPhotoTexture.reset();
        mCheckerboardTexture.reset();
        mDefaultSampler.reset();
        mCubeMesh.reset();
        mCmdBuffers.clear();
        mViewportRenderTarget.reset();
        mViewportRenderPass.reset();
        mUiRenderTarget.reset();
        mUiRenderPass.reset();
        mRenderer.reset();
    }

    void CmEditorApp::UpdateSwapchainTarget() {
        CmRenderContext *renderContext = CmApplication::GetAppContext()->renderCxt;
        CmVKDevice *device = renderContext->GetDevice();
        CmVKSwapchain *swapchain = renderContext->GetSwapchain();
        uint32_t imageCount = static_cast<uint32_t>(swapchain->GetImages().size());
        uint32_t width = swapchain->GetWidth();
        uint32_t height = swapchain->GetHeight();

        if(mUiRenderTarget && (mTargetWidth != width || mTargetHeight != height)){
            mUiRenderTarget->SetExtent({ width, height });
            mTargetWidth = width;
            mTargetHeight = height;
        }
        if(mUiRenderTarget && mTargetImageCount != imageCount){
            mUiRenderTarget->SetBufferCount(imageCount);
            mTargetImageCount = imageCount;
        }
        if(mCmdBuffers.size() != imageCount){
            mCmdBuffers = device->GetDefaultCmdPool()->AllocateCommandBuffer(imageCount);
        }
    }

    void CmEditorApp::UpdateViewportTarget() {
        if(!mEditorContext || !mViewportRenderTarget){
            return;
        }

        uint32_t width = std::max(64u, mEditorContext->GetViewportWidth());
        uint32_t height = std::max(64u, mEditorContext->GetViewportHeight());
        if(width == mViewportTargetWidth && height == mViewportTargetHeight){
            return;
        }

        CmRenderContext *renderContext = CmApplication::GetAppContext()->renderCxt;
        CmVKDevice *device = renderContext->GetDevice();
        vkDeviceWaitIdle(device->GetHandle());

        mEditorContext->SetViewportTextureId(0);
        for(uint64_t textureId : mViewportTextureIds){
            if(mImGuiLayer){
                mImGuiLayer->RemoveTexture(textureId);
            }
        }
        mViewportTextureIds.clear();
        mViewportImageViews.clear();

        mViewportRenderTarget->SetExtent({ width, height });
        mViewportTargetWidth = width;
        mViewportTargetHeight = height;
    }

    void CmEditorApp::UpdateViewportTexture() {
        if(!mViewportRenderTarget || !mImGuiLayer || !mEditorContext){
            return;
        }

        CmVKImageView *imageView = mViewportRenderTarget->GetImageView(mViewportResolveAttachmentIndex);
        if(!imageView){
            return;
        }

        VkImageView viewHandle = imageView->GetHandle();
        for(uint32_t i = 0; i < mViewportImageViews.size(); i++){
            if(mViewportImageViews[i] == viewHandle){
                mEditorContext->SetViewportTextureId(mViewportTextureIds[i]);
                return;
            }
        }

        uint64_t textureId = mImGuiLayer->AddTexture(viewHandle, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        if(textureId != 0){
            mViewportImageViews.push_back(viewHandle);
            mViewportTextureIds.push_back(textureId);
            mEditorContext->SetViewportTextureId(textureId);
        }
    }

    void CmEditorApp::UpdateEditorCamera(float deltaTime) {
        if(!mEditorContext || !mWindow || !mViewportRenderTarget || !mEditorContext->IsViewportHovered()){
            bFirstCameraDrag = true;
            return;
        }

        bool orbiting = mWindow->IsMouseDown(MOUSE_BUTTON_RIGHT);
        bool panning = mWindow->IsMouseDown(MOUSE_BUTTON_MIDDLE);
        if(!orbiting && !panning){
            bFirstCameraDrag = true;
            return;
        }

        CmEntity *camera = mViewportRenderTarget->GetCamera();
        if(!CmEntity::HasComponent<CmLookAtCameraComponent>(camera) || !camera->HasComponent<CmTransformComponent>()){
            bFirstCameraDrag = true;
            return;
        }

        glm::vec2 mousePos;
        mWindow->GetMousePos(mousePos);
        if(bFirstCameraDrag){
            mLastCameraMousePos = mousePos;
            bFirstCameraDrag = false;
            return;
        }

        glm::vec2 mouseDelta = mousePos - mLastCameraMousePos;
        mLastCameraMousePos = mousePos;
        if(std::abs(mouseDelta.x) < 0.1f && std::abs(mouseDelta.y) < 0.1f){
            return;
        }

        auto &cameraComp = camera->GetComponent<CmLookAtCameraComponent>();
        auto &transform = camera->GetComponent<CmTransformComponent>();

        if(orbiting){
            transform.rotation.x -= mouseDelta.x * mCameraOrbitSensitivity;
            transform.rotation.y += mouseDelta.y * mCameraOrbitSensitivity;
            transform.rotation.y = std::clamp(transform.rotation.y, -89.f, 89.f);
            return;
        }

        float yaw = glm::radians(transform.rotation.x);
        float pitch = glm::radians(transform.rotation.y);
        glm::vec3 targetToCamera = {
            std::cos(pitch) * std::sin(yaw),
            std::sin(pitch),
            std::cos(pitch) * std::cos(yaw)
        };
        glm::vec3 forward = glm::normalize(-targetToCamera);
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3{ 0.f, 1.f, 0.f }));
        glm::vec3 up = glm::normalize(glm::cross(right, forward));
        float panScale = cameraComp.GetRadius() * mCameraPanSensitivity * std::max(deltaTime * 60.f, 0.1f);

        cameraComp.SetTarget(cameraComp.GetTarget() + (-right * mouseDelta.x + up * mouseDelta.y) * panScale);
    }

    void CmEditorApp::ZoomEditorCamera(float offset) {
        if(!mViewportRenderTarget){
            return;
        }

        CmEntity *camera = mViewportRenderTarget->GetCamera();
        if(!CmEntity::HasComponent<CmLookAtCameraComponent>(camera)){
            return;
        }

        auto &cameraComp = camera->GetComponent<CmLookAtCameraComponent>();
        float radius = cameraComp.GetRadius() - offset * mCameraZoomSensitivity;
        cameraComp.SetRadius(std::clamp(radius, 0.3f, 100.f));
    }
}
