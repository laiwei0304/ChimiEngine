#include "renderer/SceneExtraction.h"

#include "core/Assert.h"

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>

namespace chimi::renderer
{
namespace
{
glm::mat4 BuildViewProjection(const chimi::scene::Camera& camera)
{
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.verticalFovDegrees),
        camera.aspectRatio,
        camera.nearPlane,
        camera.farPlane);
    projection[1][1] *= -1.0f;

    const glm::mat4 view = glm::lookAt(
        camera.position,
        camera.target,
        camera.up);
    return projection * view;
}

glm::vec3 NormalizeOrDefault(glm::vec3 value, glm::vec3 fallback)
{
    if (glm::length(value) > 0.0001f)
    {
        return glm::normalize(value);
    }

    return fallback;
}
}

RenderPacket ExtractSceneRenderPacket(const chimi::scene::Scene& scene, const MeshLibrary& meshLibrary)
{
    RenderPacket renderPacket{};
    const chimi::scene::Camera& mainCamera = scene.GetMainCamera();
    const chimi::scene::DirectionalLight& mainDirectionalLight = scene.GetMainDirectionalLight();
    const std::vector<chimi::scene::MeshRenderNode>& meshRenderNodes = scene.GetMeshRenderNodes();
    renderPacket.mainMeshPass.draws.reserve(meshRenderNodes.size());

    const glm::mat4 viewProjection = BuildViewProjection(mainCamera);
    const glm::vec3 worldLightDirection = NormalizeOrDefault(
        mainDirectionalLight.direction,
        glm::vec3(-0.6f, -1.0f, -0.35f));

    for (const chimi::scene::MeshRenderNode& meshRenderNode : meshRenderNodes)
    {
        if (!meshRenderNode.meshRenderer.visible)
        {
            continue;
        }

        const CpuMeshData* meshData = meshLibrary.FindMesh(meshRenderNode.meshRenderer.meshId);
        CHIMI_ASSERT(meshData != nullptr, "Scene mesh node must reference a known meshId");
        const glm::mat4 objectToWorld = meshRenderNode.transform.ToMatrix();
        const glm::mat3 worldToObjectRotation = glm::transpose(glm::mat3(objectToWorld));
        const glm::vec3 objectLightDirection = NormalizeOrDefault(
            worldToObjectRotation * worldLightDirection,
            glm::vec3(-0.6f, -1.0f, -0.35f));
        renderPacket.mainMeshPass.draws.push_back(MeshDrawPacket{
            .meshId = meshRenderNode.meshRenderer.meshId,
            .meshData = meshData,
            .objectToClip = viewProjection * objectToWorld,
            .lightDirectionAndIntensity = glm::vec4(objectLightDirection, mainDirectionalLight.intensity),
            .lightColorAndAmbient = glm::vec4(mainDirectionalLight.color, mainDirectionalLight.ambient)
        });
    }

    return renderPacket;
}
}
