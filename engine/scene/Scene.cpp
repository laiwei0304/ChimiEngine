#include "scene/Scene.h"

namespace chimi::scene
{
Camera& Scene::GetMainCamera()
{
    return m_mainCamera;
}

const Camera& Scene::GetMainCamera() const
{
    return m_mainCamera;
}

DirectionalLight& Scene::GetMainDirectionalLight()
{
    return m_mainDirectionalLight;
}

const DirectionalLight& Scene::GetMainDirectionalLight() const
{
    return m_mainDirectionalLight;
}

MeshRenderNode& Scene::AddMeshRenderNode(uint64_t meshId, MeshRenderNodeDesc desc)
{
    desc.meshRenderer.meshId = meshId;
    m_meshRenderNodes.push_back(MeshRenderNode{
        .nodeId = m_nextNodeId++,
        .transform = desc.transform,
        .meshRenderer = desc.meshRenderer
    });
    return m_meshRenderNodes.back();
}

MeshRenderNode* Scene::FindMeshRenderNode(SceneNodeId nodeId)
{
    for (MeshRenderNode& meshRenderNode : m_meshRenderNodes)
    {
        if (meshRenderNode.nodeId == nodeId)
        {
            return &meshRenderNode;
        }
    }

    return nullptr;
}

const MeshRenderNode* Scene::FindMeshRenderNode(SceneNodeId nodeId) const
{
    for (const MeshRenderNode& meshRenderNode : m_meshRenderNodes)
    {
        if (meshRenderNode.nodeId == nodeId)
        {
            return &meshRenderNode;
        }
    }

    return nullptr;
}

bool Scene::SetMeshRenderNodeTransform(SceneNodeId nodeId, const Transform& transform)
{
    if (MeshRenderNode* meshRenderNode = FindMeshRenderNode(nodeId))
    {
        meshRenderNode->transform = transform;
        return true;
    }

    return false;
}

bool Scene::SetMeshRenderNodeVisibility(SceneNodeId nodeId, bool visible)
{
    if (MeshRenderNode* meshRenderNode = FindMeshRenderNode(nodeId))
    {
        meshRenderNode->meshRenderer.visible = visible;
        return true;
    }

    return false;
}

bool Scene::RemoveMeshRenderNode(SceneNodeId nodeId)
{
    for (auto it = m_meshRenderNodes.begin(); it != m_meshRenderNodes.end(); ++it)
    {
        if (it->nodeId == nodeId)
        {
            m_meshRenderNodes.erase(it);
            return true;
        }
    }

    return false;
}

const std::vector<MeshRenderNode>& Scene::GetMeshRenderNodes() const
{
    return m_meshRenderNodes;
}
}
