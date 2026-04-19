#pragma once

#include "renderer/MeshLibrary.h"
#include "renderer/RenderPacket.h"
#include "scene/Scene.h"

namespace chimi::renderer
{
class Renderer
{
public:
    explicit Renderer(const MeshLibrary& meshLibrary);

    RenderPacket BuildRenderPacket(const chimi::scene::Scene& scene) const;

private:
    const MeshLibrary* m_meshLibrary = nullptr;
};
}
