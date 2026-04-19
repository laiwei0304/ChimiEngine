#include "renderer/Renderer.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace chimi::renderer
{
namespace
{
constexpr uint64_t kSampleCubeMeshId = 1;
}

Renderer::Renderer()
{
    m_sampleMeshData.vertices = {
        Vertex{ .position = { -0.5f, -0.5f, 0.5f }, .color = { 1.0f, 0.2f, 0.2f } },
        Vertex{ .position = { 0.5f, -0.5f, 0.5f }, .color = { 0.2f, 1.0f, 0.2f } },
        Vertex{ .position = { 0.5f, 0.5f, 0.5f }, .color = { 0.2f, 0.4f, 1.0f } },
        Vertex{ .position = { -0.5f, 0.5f, 0.5f }, .color = { 1.0f, 1.0f, 0.2f } },
        Vertex{ .position = { -0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.2f, 1.0f } },
        Vertex{ .position = { 0.5f, -0.5f, -0.5f }, .color = { 0.2f, 1.0f, 1.0f } },
        Vertex{ .position = { 0.5f, 0.5f, -0.5f }, .color = { 1.0f, 0.6f, 0.2f } },
        Vertex{ .position = { -0.5f, 0.5f, -0.5f }, .color = { 0.6f, 0.2f, 1.0f } }
    };

    m_sampleMeshData.indices = {
        0, 1, 2, 2, 3, 0,
        1, 5, 6, 6, 2, 1,
        5, 4, 7, 7, 6, 5,
        4, 0, 3, 3, 7, 4,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1280.0f / 720.0f, 0.1f, 10.0f);
    projection[1][1] *= -1.0f;

    const glm::mat4 view = glm::lookAt(
        glm::vec3(1.6f, 1.2f, 2.4f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    m_cameraData.viewProjection = projection * view;
}

RenderPacket Renderer::BuildRenderPacket() const
{
    RenderPacket renderPacket{};
    renderPacket.mainMeshPass.draws.push_back(MeshDrawPacket{
        .meshId = kSampleCubeMeshId,
        .meshData = &m_sampleMeshData,
        .objectToClip = m_cameraData.viewProjection * glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f, 0.0f, 0.0f))
    });
    renderPacket.mainMeshPass.draws.push_back(MeshDrawPacket{
        .meshId = kSampleCubeMeshId,
        .meshData = &m_sampleMeshData,
        .objectToClip = m_cameraData.viewProjection * glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, 0.0f, 0.0f))
    });
    return renderPacket;
}
}
