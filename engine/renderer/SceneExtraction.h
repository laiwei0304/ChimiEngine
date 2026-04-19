#pragma once

#include "renderer/MeshLibrary.h"
#include "renderer/RenderPacket.h"
#include "scene/Scene.h"

namespace chimi::renderer
{
RenderPacket ExtractSceneRenderPacket(const chimi::scene::Scene& scene, const MeshLibrary& meshLibrary);
}
