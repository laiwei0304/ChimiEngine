#pragma once
#include "CmGraphicContext.h"
#include "Graphic/CmVKCommon.h"
#include "ECS/CmSystem.h"

namespace chimi{
    class CmVKRenderPass;
    class CmRenderTarget;
    class CmApplication;
    class CmScene;
    class CmVKDevice;

    class CmMaterialSystem : public CmSystem{
    public:
        virtual void OnInit(CmVKRenderPass *renderPass) = 0;
        virtual void OnRender(VkCommandBuffer cmdBuffer, CmRenderTarget *renderTarget) = 0;
        virtual void OnDestroy() = 0;
    protected:
        CmApplication *GetApp() const;
        CmScene *GetScene() const;
        CmVKDevice *GetDevice() const;
        const glm::mat4 GetProjMat(CmRenderTarget *renderTarget) const;
        const glm::mat4 GetViewMat(CmRenderTarget *renderTarget) const;
    };
}