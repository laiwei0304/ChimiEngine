#pragma once

#include "scene/Scene.h"

#include <cstdint>

namespace chimi::app::sandbox
{
struct SandboxSceneBundle
{
    chimi::scene::Scene scene{};
    chimi::scene::SceneNodeId leftCubeNodeId = 0;
    chimi::scene::SceneNodeId rightCubeNodeId = 0;
    chimi::scene::SceneNodeId transientCubeNodeId = 0;
    bool transientCubeRemoved = false;
};

SandboxSceneBundle CreateSandboxSampleScene(uint64_t sampleMeshId);
}
