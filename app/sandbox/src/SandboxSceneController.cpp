#include "SandboxSceneController.h"

#include <cmath>

namespace chimi::app::sandbox
{
namespace
{
bool UpdateCubeRotation(chimi::scene::Scene& scene, chimi::scene::SceneNodeId nodeId, float rotationDegreesY)
{
    if (const chimi::scene::MeshRenderNode* meshRenderNode = scene.FindMeshRenderNode(nodeId))
    {
        chimi::scene::Transform updatedTransform = meshRenderNode->transform;
        updatedTransform.rotationDegrees.y = rotationDegreesY;
        return scene.SetMeshRenderNodeTransform(nodeId, updatedTransform);
    }

    return false;
}
}

void UpdateSandboxScene(SandboxSceneBundle& sandboxScene, float elapsedSeconds, float aspectRatio)
{
    UpdateCubeRotation(sandboxScene.scene, sandboxScene.leftCubeNodeId, -18.0f + elapsedSeconds * 45.0f);
    UpdateCubeRotation(sandboxScene.scene, sandboxScene.rightCubeNodeId, 18.0f - elapsedSeconds * 45.0f);

    sandboxScene.scene.SetMeshRenderNodeVisibility(
        sandboxScene.rightCubeNodeId,
        std::sin(elapsedSeconds * 1.6f) > -0.15f);

    if (!sandboxScene.transientCubeRemoved && elapsedSeconds > 6.0f)
    {
        sandboxScene.transientCubeRemoved =
            sandboxScene.scene.RemoveMeshRenderNode(sandboxScene.transientCubeNodeId);
    }
    else if (!sandboxScene.transientCubeRemoved)
    {
        UpdateCubeRotation(sandboxScene.scene, sandboxScene.transientCubeNodeId, elapsedSeconds * 90.0f);
    }

    chimi::scene::Camera& mainCamera = sandboxScene.scene.GetMainCamera();
    mainCamera.aspectRatio = aspectRatio;
    mainCamera.position = { 1.6f, 1.2f, 2.4f };
    mainCamera.target = { 0.0f, 0.0f, 0.0f };
    mainCamera.up = { 0.0f, 1.0f, 0.0f };

    chimi::scene::DirectionalLight& mainDirectionalLight = sandboxScene.scene.GetMainDirectionalLight();
    mainDirectionalLight.direction = {
        -0.55f + std::cos(elapsedSeconds * 0.45f) * 0.25f,
        -1.0f,
        -0.25f + std::sin(elapsedSeconds * 0.45f) * 0.35f
    };
    mainDirectionalLight.intensity = 1.2f + std::sin(elapsedSeconds * 0.7f) * 0.15f;
}
}
