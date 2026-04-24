#pragma once

#include "CmUUID.h"
#include "entt/entity/registry.hpp"

namespace chimi{
    class CmNode;
    class CmEntity;

    class CmScene{
    public:
        CmScene();
        ~CmScene();

        CmEntity* CreateEntity(const std::string &name = "");
        CmEntity* CreateEntityWithUUID(const CmUUID &id, const std::string &name = "");
        void DestroyEntity(const CmEntity *entity);
        void DestroyAllEntity();

        entt::registry &GetEcsRegistry() { return mEcsRegistry; }
        CmNode *GetRootNode() const { return mRootNode.get(); }
        CmEntity *GetEntity(entt::entity enttEntity);
    private:
        std::string mName;
        entt::registry mEcsRegistry;

        std::unordered_map<entt::entity, std::shared_ptr<CmEntity>> mEntities;
        std::shared_ptr<CmNode> mRootNode;

        friend class CmEntity;
    };
}
