#pragma once

#include <cstdint>
#include <vector>

#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace chimi::scene
{
using SceneNodeId = uint32_t;

struct Transform
{
    glm::vec3 translation{ 0.0f, 0.0f, 0.0f };
    glm::vec3 rotationDegrees{ 0.0f, 0.0f, 0.0f };
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 ToMatrix() const
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation);
        transform = glm::rotate(transform, glm::radians(rotationDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(rotationDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(rotationDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, scale);
        return transform;
    }
};

struct MeshRenderer
{
    uint64_t meshId = 0;
    bool visible = true;
};

struct Camera
{
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 target{ 0.0f, 0.0f, 0.0f };
    glm::vec3 up{ 0.0f, 1.0f, 0.0f };
    float verticalFovDegrees = 60.0f;
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 10.0f;
};

struct DirectionalLight
{
    glm::vec3 direction{ -0.6f, -1.0f, -0.35f };
    float intensity = 1.0f;
    glm::vec3 color{ 1.0f, 0.98f, 0.92f };
    float ambient = 0.18f;
};

struct MeshRenderNode
{
    SceneNodeId nodeId = 0;
    Transform transform{};
    MeshRenderer meshRenderer{};
};

struct MeshRenderNodeDesc
{
    Transform transform{};
    MeshRenderer meshRenderer{};
};

class Scene
{
public:
    Camera& GetMainCamera();
    const Camera& GetMainCamera() const;
    DirectionalLight& GetMainDirectionalLight();
    const DirectionalLight& GetMainDirectionalLight() const;

    MeshRenderNode& AddMeshRenderNode(uint64_t meshId, MeshRenderNodeDesc desc = {});
    MeshRenderNode* FindMeshRenderNode(SceneNodeId nodeId);
    const MeshRenderNode* FindMeshRenderNode(SceneNodeId nodeId) const;
    bool SetMeshRenderNodeTransform(SceneNodeId nodeId, const Transform& transform);
    bool SetMeshRenderNodeVisibility(SceneNodeId nodeId, bool visible);
    bool RemoveMeshRenderNode(SceneNodeId nodeId);
    const std::vector<MeshRenderNode>& GetMeshRenderNodes() const;

private:
    Camera m_mainCamera{};
    DirectionalLight m_mainDirectionalLight{};
    std::vector<MeshRenderNode> m_meshRenderNodes;
    SceneNodeId m_nextNodeId = 1;
};
}
