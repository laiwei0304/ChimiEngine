#pragma once

#include "CmNode.h"
#include "CmScene.h"

namespace chimi{
    class CmEntity : public CmNode {
    public:
        CmEntity(const entt::entity &ecsEntity, CmScene *scene) : mEcsEntity(ecsEntity), mScene(scene) {}
        ~CmEntity() override = default;

        static bool IsValid(CmEntity *entity){
            return entity && entity->IsValid();
        }

        template<typename T>
        static bool HasComponent(CmEntity *entity){
            return IsValid(entity) && entity->HasComponent<T>();
        }

        bool operator==(const CmEntity& other) const{
            return mEcsEntity == other.mEcsEntity && mScene == other.mScene;
        }
        bool operator!=(const CmEntity& other) const{
            return !(*this == other);
        }

        bool IsValid() const { return mScene && mScene->mEcsRegistry.valid(mEcsEntity); }
        const entt::entity &GetEcsEntity() const { return mEcsEntity; }

        template<typename T, typename... Args>
        T& AddComponent(Args &&...args){
            T &component = mScene->mEcsRegistry.emplace<T>(mEcsEntity, std::forward<Args>(args)...);
            component.SetOwner(this);
            return component;
        }

        template<typename T>
        bool HasComponent(){
            return mScene->mEcsRegistry.any_of<T>(mEcsEntity);
        }

        template<typename... T>
        bool HasAnyComponent() {
            return mScene->mEcsRegistry.any_of<T...>(mEcsEntity);
        }

        template<typename... T>
        bool HasAllComponent(){
            return mScene->mEcsRegistry.all_of<T...>(mEcsEntity);
        }

        template<typename T>
        T& GetComponent(){
            assert(HasComponent<T>() && "Entity does not have component!");
            return mScene->mEcsRegistry.get<T>(mEcsEntity);
        }

        template<typename T>
        void RemoveComponent(){
            assert(HasComponent<T>() && "Entity does not have component!");
            mScene->mEcsRegistry.remove<T>(mEcsEntity);
        }

    private:
        entt::entity mEcsEntity;
        CmScene *mScene;
    };
}
