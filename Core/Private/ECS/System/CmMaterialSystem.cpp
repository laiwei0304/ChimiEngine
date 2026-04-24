#include "ECS/System/CmMaterialSystem.h"

#include "CmApplication.h"
#include "Render/CmRenderContext.h"
#include "Render/CmRenderTarget.h"
#include "ECS/Component/CmLookAtCameraComponent.h"

namespace chimi{
    CmApplication *CmMaterialSystem::GetApp() const {
        CmAppContext *appContext = CmApplication::GetAppContext();
        if(appContext){
            return appContext->app;
        }
        return nullptr;
    }

    CmScene *CmMaterialSystem::GetScene() const {
        CmAppContext *appContext = CmApplication::GetAppContext();
        if(appContext){
            return appContext->scene;
        }
        return nullptr;
    }

    CmVKDevice *CmMaterialSystem::GetDevice() const {
        CmAppContext *appContext = CmApplication::GetAppContext();
        if(appContext){
            if(appContext->renderCxt){
                return appContext->renderCxt->GetDevice();
            }
        }
        return nullptr;
    }

    const glm::mat4 CmMaterialSystem::GetProjMat(CmRenderTarget *renderTarget) const {
        glm::mat4 projMat{1.f};
        CmEntity *camera = renderTarget->GetCamera();
        if(CmEntity::HasComponent<CmLookAtCameraComponent>(camera)){
            auto &cameraComp = camera->GetComponent<CmLookAtCameraComponent>();
            projMat = cameraComp.GetProjMat();
        }
        return projMat;
    }

    const glm::mat4 CmMaterialSystem::GetViewMat(CmRenderTarget *renderTarget) const {
        glm::mat4 viewMat{1.f};
        CmEntity *camera = renderTarget->GetCamera();
        if(CmEntity::HasComponent<CmLookAtCameraComponent>(camera)){
            auto &cameraComp = camera->GetComponent<CmLookAtCameraComponent>();
            viewMat = cameraComp.GetViewMat();
        }
        return viewMat;
    }
}