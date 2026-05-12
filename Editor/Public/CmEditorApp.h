#pragma once

#include "CmApplication.h"
#include "CmEditorContext.h"
#include "CmEditorLayer.h"
#include "CmImGuiLayer.h"
#include "Render/CmRenderer.h"
#include "Render/CmRenderTarget.h"
#include "Render/CmMesh.h"
#include "Render/CmTexture.h"
#include "Render/CmSampler.h"
#include "Render/CmMaterial.h"
#include "Graphic/CmVKRenderPass.h"
#include "Event/CmEventObserver.h"
#include "ECS/Component/Material/CmUnlitMaterialComponent.h"

namespace chimi{
    class CmEditorApp : public CmApplication{
    protected:
        void OnConfiguration(AppSettings *appSettings) override;
        void OnInit() override;
        void OnSceneInit(CmScene *scene) override;
        void OnSceneDestroy(CmScene *scene) override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnDestroy() override;
    private:
        void UpdateSwapchainTarget();
        void UpdateViewportTarget();
        void UpdateViewportTexture();
        void UpdateEditorCamera(float deltaTime);
        void ZoomEditorCamera(float offset);

        std::shared_ptr<CmVKRenderPass> mUiRenderPass;
        std::shared_ptr<CmRenderTarget> mUiRenderTarget;
        std::shared_ptr<CmVKRenderPass> mViewportRenderPass;
        std::shared_ptr<CmRenderTarget> mViewportRenderTarget;
        std::shared_ptr<CmRenderer> mRenderer;
        std::vector<VkCommandBuffer> mCmdBuffers;

        std::shared_ptr<CmEditorContext> mEditorContext;
        std::shared_ptr<CmEditorLayer> mEditorLayer;
        std::shared_ptr<CmImGuiLayer> mImGuiLayer;
        std::shared_ptr<CmEventObserver> mEventObserver;

        float mDeltaTime = 0.f;
        bool bFirstCameraDrag = true;
        glm::vec2 mLastCameraMousePos{ 0.f, 0.f };
        float mCameraOrbitSensitivity = 0.25f;
        float mCameraPanSensitivity = 0.0015f;
        float mCameraZoomSensitivity = 0.3f;
        uint32_t mTargetWidth = 0;
        uint32_t mTargetHeight = 0;
        uint32_t mTargetImageCount = 0;

        uint32_t mViewportTargetWidth = 0;
        uint32_t mViewportTargetHeight = 0;
        uint32_t mViewportResolveAttachmentIndex = 0;
        std::vector<VkImageView> mViewportImageViews;
        std::vector<uint64_t> mViewportTextureIds;

        std::shared_ptr<CmMesh> mCubeMesh;
        std::shared_ptr<CmSampler> mDefaultSampler;
        std::shared_ptr<CmTexture> mCheckerboardTexture;
        std::shared_ptr<CmTexture> mPhotoTexture;
        std::vector<CmUnlitMaterial*> mViewportMaterials;
    };
}
