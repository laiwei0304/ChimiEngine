#include "ECS/CmScene.h"
#include "ECS/CmEntity.h"
#include "ECS/Component/CmTransformComponent.h"

namespace chimi{
    CmScene::CmScene() {
        mRootNode = std::make_shared<CmNode>();
    }

    CmScene::~CmScene() {
        mRootNode.reset();
        DestroyAllEntity();
        mEntities.clear();
    }

    CmEntity *CmScene::CreateEntity(const std::string &name) {
        return CreateEntityWithUUID(CmUUID(), name);
    }

    CmEntity *CmScene::CreateEntityWithUUID(const CmUUID &id, const std::string &name) {
        auto enttEntity = mEcsRegistry.create();
        mEntities.insert({ enttEntity, std::make_shared<CmEntity>(enttEntity, this) });
        mEntities[enttEntity]->SetParent(mRootNode.get());
        mEntities[enttEntity]->SetId(id);
        mEntities[enttEntity]->SetName(name.empty() ? "Entity" : name);

        // add default components
        mEntities[enttEntity]->AddComponent<CmTransformComponent>();

        return mEntities[enttEntity].get();
    }

    void CmScene::DestroyEntity(const CmEntity *entity) {
        if(entity && entity->IsValid()){
            mEcsRegistry.destroy(entity->GetEcsEntity());
        }

        auto it = mEntities.find(entity->GetEcsEntity());
        if(it != mEntities.end()){
            CmNode *parent = it->second->GetParent();
            if(parent){
                parent->RemoveChild(it->second.get());
            }
            mEntities.erase(it);
        }
    }

    void CmScene::DestroyAllEntity() {
        mEcsRegistry.clear();
        mEntities.clear();
    }

    CmEntity *CmScene::GetEntity(entt::entity enttEntity) {
        if(mEntities.find(enttEntity) != mEntities.end()){
            return mEntities.at(enttEntity).get();
        }
        return nullptr;
    }
}