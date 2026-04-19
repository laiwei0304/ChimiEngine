#pragma once

#include "renderer/MeshData.h"
#include "renderer/RenderPacket.h"

namespace chimi::renderer
{
struct CameraData
{
    glm::mat4 viewProjection{ 1.0f };
};

class Renderer
{
public:
    Renderer();

    RenderPacket BuildRenderPacket() const;

private:
    CpuMeshData m_sampleMeshData{};
    CameraData m_cameraData{};
};
}
