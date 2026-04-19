#pragma once

#include "renderer/RenderPacket.h"
#include "rhi/vulkan/VulkanMeshBuffer.h"

#include <functional>
#include <vector>

namespace chimi::rhi::vulkan
{
struct MeshGpuResource
{
    uint64_t meshId = 0;
    MeshBuffer meshBuffer{};
};

struct VulkanMeshCache
{
    std::vector<MeshGpuResource> meshGpuResources;
};

using CreateMeshBufferFn = std::function<MeshBuffer(const chimi::renderer::CpuMeshData&)>;
using DestroyMeshBufferFn = std::function<void(MeshBuffer&)>;

const MeshGpuResource* FindMeshGpuResource(const VulkanMeshCache& meshCache, uint64_t meshId);
void SyncMeshCache(
    VulkanMeshCache& meshCache,
    const chimi::renderer::MeshPassPacket& meshPass,
    const CreateMeshBufferFn& createMeshBuffer,
    const DestroyMeshBufferFn& destroyMeshBuffer);
void DestroyMeshCache(VulkanMeshCache& meshCache, const DestroyMeshBufferFn& destroyMeshBuffer);
}
