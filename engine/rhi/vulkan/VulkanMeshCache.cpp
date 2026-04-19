#include "rhi/vulkan/VulkanMeshCache.h"

#include "core/Assert.h"

namespace chimi::rhi::vulkan
{
const MeshGpuResource* FindMeshGpuResource(const VulkanMeshCache& meshCache, uint64_t meshId)
{
    for (const MeshGpuResource& resource : meshCache.meshGpuResources)
    {
        if (resource.meshId == meshId)
        {
            return &resource;
        }
    }

    return nullptr;
}

namespace
{
std::vector<const chimi::renderer::MeshDrawPacket*> CollectUniqueMeshDraws(
    const chimi::renderer::MeshPassPacket& meshPass)
{
    std::vector<const chimi::renderer::MeshDrawPacket*> uniqueMeshDraws;
    uniqueMeshDraws.reserve(meshPass.draws.size());

    for (const chimi::renderer::MeshDrawPacket& draw : meshPass.draws)
    {
        bool alreadyAdded = false;
        for (const chimi::renderer::MeshDrawPacket* uniqueDraw : uniqueMeshDraws)
        {
            if (uniqueDraw->meshId == draw.meshId)
            {
                alreadyAdded = true;
                break;
            }
        }

        if (!alreadyAdded)
        {
            uniqueMeshDraws.push_back(&draw);
        }
    }

    return uniqueMeshDraws;
}
}

void SyncMeshCache(
    VulkanMeshCache& meshCache,
    const chimi::renderer::MeshPassPacket& meshPass,
    const CreateMeshBufferFn& createMeshBuffer,
    const DestroyMeshBufferFn& destroyMeshBuffer)
{
    CHIMI_ASSERT(!meshPass.draws.empty(), "RenderPacket main mesh pass must contain at least one draw");

    const std::vector<const chimi::renderer::MeshDrawPacket*> uniqueMeshDraws = CollectUniqueMeshDraws(meshPass);

    bool needsRebuild = meshCache.meshGpuResources.size() != uniqueMeshDraws.size();
    if (!needsRebuild)
    {
        for (const chimi::renderer::MeshDrawPacket* uniqueDraw : uniqueMeshDraws)
        {
            if (FindMeshGpuResource(meshCache, uniqueDraw->meshId) == nullptr)
            {
                needsRebuild = true;
                break;
            }
        }
    }

    if (!needsRebuild)
    {
        return;
    }

    DestroyMeshCache(meshCache, destroyMeshBuffer);
    meshCache.meshGpuResources.reserve(uniqueMeshDraws.size());

    for (const chimi::renderer::MeshDrawPacket* uniqueDraw : uniqueMeshDraws)
    {
        CHIMI_ASSERT(uniqueDraw->meshId != 0, "MeshDrawPacket must contain a valid meshId");
        CHIMI_ASSERT(uniqueDraw->meshData != nullptr, "MeshDrawPacket must contain mesh data");
        meshCache.meshGpuResources.push_back(MeshGpuResource{
            .meshId = uniqueDraw->meshId,
            .meshBuffer = createMeshBuffer(*uniqueDraw->meshData)
        });
    }
}

void DestroyMeshCache(VulkanMeshCache& meshCache, const DestroyMeshBufferFn& destroyMeshBuffer)
{
    for (MeshGpuResource& resource : meshCache.meshGpuResources)
    {
        destroyMeshBuffer(resource.meshBuffer);
        resource.meshBuffer.indexCount = 0;
    }

    meshCache.meshGpuResources.clear();
}
}
