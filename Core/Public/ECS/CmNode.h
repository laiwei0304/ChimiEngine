#pragma once

#include "CmUUID.h"

namespace chimi{
    class CmNode{
    public:
        CmNode() = default;
        virtual ~CmNode() = default;

        CmUUID GetId() const;
        void SetId(const CmUUID &nodeId);
        const std::string &GetName() const;
        void SetName(const std::string &name);

        const std::vector<CmNode *> &GetChildren() const;
        bool HasParent();
        bool HasChildren();
        void SetParent(CmNode *node);
        CmNode *GetParent() const;
        void AddChild(CmNode *node);
        void RemoveChild(CmNode *node);
    private:
        CmUUID mId;
        std::string mName;
        CmNode *mParent = nullptr;
        std::vector<CmNode*> mChildren;
    };
}
