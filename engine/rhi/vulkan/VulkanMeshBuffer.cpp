#include "rhi/vulkan/VulkanMeshBuffer.h"

#include "core/Assert.h"

#include <volk.h>

namespace chimi::rhi::vulkan
{
MeshBuffer CreateMeshBuffer(
    const VulkanContext& context,
    const VulkanUploadContext& uploadContext,
    const chimi::renderer::CpuMeshData& meshData,
    const MeshBufferFindMemoryTypeFn& findMemoryType)
{
    CHIMI_ASSERT(!meshData.vertices.empty(), "CpuMeshData must contain vertices");
    CHIMI_ASSERT(!meshData.indices.empty(), "CpuMeshData must contain indices");

    MeshBuffer meshBuffer{};
    UploadBufferWithStaging(
        uploadContext,
        context.device,
        context.graphicsQueue,
        meshData.vertices.data(),
        static_cast<VkDeviceSize>(meshData.vertices.size() * sizeof(chimi::renderer::Vertex)),
        meshBuffer.vertexBuffer,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        findMemoryType);

    UploadBufferWithStaging(
        uploadContext,
        context.device,
        context.graphicsQueue,
        meshData.indices.data(),
        static_cast<VkDeviceSize>(meshData.indices.size() * sizeof(uint32_t)),
        meshBuffer.indexBuffer,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        findMemoryType);

    meshBuffer.indexCount = static_cast<uint32_t>(meshData.indices.size());
    return meshBuffer;
}

void DrawMeshBuffer(VkCommandBuffer commandBuffer, const MeshBuffer& meshBuffer)
{
    const VkDeviceSize vertexBufferOffset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshBuffer.vertexBuffer.buffer, &vertexBufferOffset);
    vkCmdBindIndexBuffer(commandBuffer, meshBuffer.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, meshBuffer.indexCount, 1, 0, 0, 0);
}

void DestroyMeshBuffer(VkDevice device, MeshBuffer& meshBuffer)
{
    chimi::rhi::vulkan::DestroyBuffer(device, meshBuffer.indexBuffer);
    chimi::rhi::vulkan::DestroyBuffer(device, meshBuffer.vertexBuffer);
    meshBuffer.indexCount = 0;
}
}
