#include "SandboxScene.h"

namespace chimi::app::sandbox
{
SandboxSceneBundle CreateSandboxSampleScene(uint64_t sampleMeshId)
{
    SandboxSceneBundle bundle{};
    chimi::scene::Camera& mainCamera = bundle.scene.GetMainCamera();
    mainCamera.position = { 1.6f, 1.2f, 2.4f };
    mainCamera.target = { 0.0f, 0.0f, 0.0f };
    mainCamera.up = { 0.0f, 1.0f, 0.0f };
    mainCamera.verticalFovDegrees = 60.0f;
    mainCamera.aspectRatio = 1280.0f / 720.0f;
    mainCamera.nearPlane = 0.1f;
    mainCamera.farPlane = 10.0f;

    chimi::scene::DirectionalLight& mainDirectionalLight = bundle.scene.GetMainDirectionalLight();
    mainDirectionalLight.direction = { -0.55f, -1.0f, -0.25f };
    mainDirectionalLight.intensity = 1.35f;
    mainDirectionalLight.color = { 1.0f, 0.95f, 0.86f };
    mainDirectionalLight.ambient = 0.22f;

    bundle.leftCubeNodeId = bundle.scene
                                .AddMeshRenderNode(
                                    sampleMeshId,
                                    chimi::scene::MeshRenderNodeDesc{
                                        .transform = chimi::scene::Transform{
                                            .translation = { -0.8f, 0.0f, 0.0f },
                                            .rotationDegrees = { 0.0f, -18.0f, 0.0f }
                                        }
                                    })
                                .nodeId;

    bundle.rightCubeNodeId = bundle.scene
                                 .AddMeshRenderNode(
                                     sampleMeshId,
                                     chimi::scene::MeshRenderNodeDesc{
                                         .transform = chimi::scene::Transform{
                                             .translation = { 0.8f, 0.0f, 0.0f },
                                             .rotationDegrees = { 0.0f, 18.0f, 0.0f }
                                         }
                                     })
                                 .nodeId;

    bundle.transientCubeNodeId = bundle.scene
                                     .AddMeshRenderNode(
                                         sampleMeshId,
                                         chimi::scene::MeshRenderNodeDesc{
                                             .transform = chimi::scene::Transform{
                                                 .translation = { 0.0f, 0.9f, 0.0f },
                                                 .scale = { 0.55f, 0.55f, 0.55f }
                                             }
                                         })
                                     .nodeId;

    return bundle;
}
}
