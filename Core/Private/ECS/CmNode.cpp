#include "ECS/CmNode.h"

namespace chimi{
    CmUUID CmNode::GetId() const {
        return mId;
    }

    void CmNode::SetId(const CmUUID &nodeId) {
        mId = nodeId;
    }

    const std::string &CmNode::GetName() const {
        return mName;
    }

    void CmNode::SetName(const std::string &name) {
        mName = name;
    }

    const std::vector<CmNode *> &CmNode::GetChildren() const {
        return mChildren;
    }

    bool CmNode::HasParent() {
        return mParent != nullptr;
    }

    bool CmNode::HasChildren() {
        return !mChildren.empty();
    }

    void CmNode::SetParent(CmNode *node) {
        node->AddChild(this);
    }

    CmNode *CmNode::GetParent() const {
        return mParent;
    }

    void CmNode::AddChild(CmNode *node) {
        if(node->HasParent()){
            node->GetParent()->RemoveChild(node);
        }
        node->mParent = this;
        mChildren.push_back(node);
    }

    void CmNode::RemoveChild(CmNode *node) {
        if(!HasChildren()){
            return;
        }
        for(auto it = mChildren.begin(); it != mChildren.end(); ++it){
            if(node == *it){
                mChildren.erase(it);
                break;
            }
        }
    }
}