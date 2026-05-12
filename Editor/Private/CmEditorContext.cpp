#include "CmEditorContext.h"

#include "ECS/CmEntity.h"

namespace chimi{
    void CmEditorContext::SetScene(CmScene *scene) {
        mScene = scene;
        if(!scene || !CmEntity::IsValid(mSelectedEntity)){
            mSelectedEntity = nullptr;
        }
    }

    void CmEditorContext::SetViewportSize(uint32_t width, uint32_t height) {
        mViewportWidth = width;
        mViewportHeight = height;
    }
}
