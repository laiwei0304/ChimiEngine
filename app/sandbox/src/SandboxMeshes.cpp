#include "SandboxMeshes.h"

namespace chimi::app::sandbox
{
chimi::renderer::CpuMeshData CreateSampleCubeMesh()
{
    chimi::renderer::CpuMeshData meshData{};
    constexpr glm::vec3 kCubeColor{ 0.82f, 0.84f, 0.88f };
    meshData.vertices = {
        chimi::renderer::Vertex{ .position = { -0.5f, -0.5f, 0.5f }, .color = kCubeColor, .normal = { 0.0f, 0.0f, 1.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, -0.5f, 0.5f }, .color = kCubeColor, .normal = { 0.0f, 0.0f, 1.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, 0.5f, 0.5f }, .color = kCubeColor, .normal = { 0.0f, 0.0f, 1.0f } },
        chimi::renderer::Vertex{ .position = { -0.5f, 0.5f, 0.5f }, .color = kCubeColor, .normal = { 0.0f, 0.0f, 1.0f } },

        chimi::renderer::Vertex{ .position = { 0.5f, -0.5f, 0.5f }, .color = kCubeColor, .normal = { 1.0f, 0.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, -0.5f, -0.5f }, .color = kCubeColor, .normal = { 1.0f, 0.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, 0.5f, -0.5f }, .color = kCubeColor, .normal = { 1.0f, 0.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, 0.5f, 0.5f }, .color = kCubeColor, .normal = { 1.0f, 0.0f, 0.0f } },

        chimi::renderer::Vertex{ .position = { 0.5f, -0.5f, -0.5f }, .color = kCubeColor, .normal = { 0.0f, 0.0f, -1.0f } },
        chimi::renderer::Vertex{ .position = { -0.5f, -0.5f, -0.5f }, .color = kCubeColor, .normal = { 0.0f, 0.0f, -1.0f } },
        chimi::renderer::Vertex{ .position = { -0.5f, 0.5f, -0.5f }, .color = kCubeColor, .normal = { 0.0f, 0.0f, -1.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, 0.5f, -0.5f }, .color = kCubeColor, .normal = { 0.0f, 0.0f, -1.0f } },

        chimi::renderer::Vertex{ .position = { -0.5f, -0.5f, -0.5f }, .color = kCubeColor, .normal = { -1.0f, 0.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { -0.5f, -0.5f, 0.5f }, .color = kCubeColor, .normal = { -1.0f, 0.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { -0.5f, 0.5f, 0.5f }, .color = kCubeColor, .normal = { -1.0f, 0.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { -0.5f, 0.5f, -0.5f }, .color = kCubeColor, .normal = { -1.0f, 0.0f, 0.0f } },

        chimi::renderer::Vertex{ .position = { -0.5f, 0.5f, 0.5f }, .color = kCubeColor, .normal = { 0.0f, 1.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, 0.5f, 0.5f }, .color = kCubeColor, .normal = { 0.0f, 1.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, 0.5f, -0.5f }, .color = kCubeColor, .normal = { 0.0f, 1.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { -0.5f, 0.5f, -0.5f }, .color = kCubeColor, .normal = { 0.0f, 1.0f, 0.0f } },

        chimi::renderer::Vertex{ .position = { -0.5f, -0.5f, -0.5f }, .color = kCubeColor, .normal = { 0.0f, -1.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, -0.5f, -0.5f }, .color = kCubeColor, .normal = { 0.0f, -1.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { 0.5f, -0.5f, 0.5f }, .color = kCubeColor, .normal = { 0.0f, -1.0f, 0.0f } },
        chimi::renderer::Vertex{ .position = { -0.5f, -0.5f, 0.5f }, .color = kCubeColor, .normal = { 0.0f, -1.0f, 0.0f } }
    };

    meshData.indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    return meshData;
}
}
