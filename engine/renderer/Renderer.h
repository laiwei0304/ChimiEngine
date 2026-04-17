#pragma once

#include "renderer/MeshData.h"

#include <glm/mat4x4.hpp>

namespace chimi::renderer
{
struct CameraData
{
    glm::mat4 viewProjection{ 1.0f };
};

struct RenderFrameInput
{
    const CpuMeshData* meshData = nullptr;
    CameraData camera{};
};

class Renderer
{
public:
    Renderer();

    RenderFrameInput GetFrameInput() const;

private:
    CpuMeshData m_sampleMeshData{};
    CameraData m_cameraData{};
};
}
