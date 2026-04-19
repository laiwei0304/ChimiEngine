#pragma once

#include "renderer/MeshData.h"

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <vector>

namespace chimi::renderer
{
enum class MeshPassPipelineKind : uint8_t
{
    SolidColor = 0
};

enum class MeshCullMode : uint8_t
{
    None = 0,
    Back
};

enum class MeshFrontFace : uint8_t
{
    CounterClockwise = 0,
    Clockwise
};

struct MeshDrawPacket
{
    uint64_t meshId = 0;
    const CpuMeshData* meshData = nullptr;
    glm::mat4 objectToClip{ 1.0f };
};

struct MeshPassPipelineConfig
{
    MeshPassPipelineKind pipelineKind = MeshPassPipelineKind::SolidColor;
    MeshCullMode cullMode = MeshCullMode::Back;
    MeshFrontFace frontFace = MeshFrontFace::Clockwise;
    bool depthTestEnabled = true;
    bool depthWriteEnabled = true;
};

struct MeshPassPacket
{
    std::vector<MeshDrawPacket> draws;
    glm::vec4 clearColor{ 0.08f, 0.12f, 0.18f, 1.0f };
    MeshPassPipelineConfig pipeline{};
};

struct RenderPacket
{
    MeshPassPacket mainMeshPass{};
};
}
