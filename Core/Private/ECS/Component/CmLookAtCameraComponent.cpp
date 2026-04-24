#include "ECS/Component/CmLookAtCameraComponent.h"
#include "ECS/Component/CmTransformComponent.h"

namespace chimi{
    const glm::mat4 &CmLookAtCameraComponent::GetProjMat() {
        mProjMat = glm::perspective(glm::radians(mFov), mAspect, mNearPlane, mFarPlane);
        mProjMat[1][1] *= -1.f;
        return mProjMat;
    }

    const glm::mat4 &CmLookAtCameraComponent::GetViewMat() {
        CmEntity *owner = GetOwner();
        if(CmEntity::HasComponent<CmTransformComponent>(owner)){
            auto &transComp = owner->GetComponent<CmTransformComponent>();
            float yaw = transComp.rotation.x;
            float pitch = transComp.rotation.y;

            glm::vec3 direction;
            direction.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
            direction.y = sin(glm::radians(pitch));
            direction.z = cos(glm::radians(pitch)) * cos(glm::radians(yaw));

            transComp.position = mTarget + direction * mRadius;

            mViewMat = glm::lookAt(transComp.position, mTarget, mWorldUp);
        }
        return mViewMat;
    }

    void CmLookAtCameraComponent::SetViewMat(const glm::mat4 &viewMat) {
        // TODO
    }
}