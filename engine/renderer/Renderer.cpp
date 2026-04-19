#include "renderer/Renderer.h"

#include "core/Assert.h"
#include "renderer/SceneExtraction.h"

namespace chimi::renderer
{
Renderer::Renderer(const MeshLibrary& meshLibrary)
    : m_meshLibrary(&meshLibrary)
{
}

RenderPacket Renderer::BuildRenderPacket(const chimi::scene::Scene& scene) const
{
    CHIMI_ASSERT(m_meshLibrary != nullptr, "Renderer must reference a valid mesh library");
    return ExtractSceneRenderPacket(scene, *m_meshLibrary);
}
}
