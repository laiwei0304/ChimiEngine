#pragma once

#include "renderer/RenderPacket.h"
#include "rhi/vulkan/VulkanMeshCache.h"
#include "rhi/vulkan/VulkanSwapchain.h"

#include <functional>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace chimi::rhi::vulkan
{
struct MeshPipelineState
{
    chimi::renderer::MeshPassPipelineConfig config{};
    bool valid = false;
};

struct PreparedMeshDraw
{
    const MeshBuffer* meshBuffer = nullptr;
    glm::mat4 objectToClip{ 1.0f };
    glm::vec4 lightDirectionAndIntensity{ -0.6f, -1.0f, -0.35f, 1.0f };
    glm::vec4 lightColorAndAmbient{ 1.0f, 0.98f, 0.92f, 0.18f };
};

struct PreparedMeshPass
{
    std::vector<PreparedMeshDraw> draws;
    float clearColor[4] = { 0.08f, 0.12f, 0.18f, 1.0f };
};

struct VulkanMeshPass
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    MeshPipelineState pipelineState{};
};

struct MeshPassCommandContext
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    uint32_t imageIndex = 0;
    VulkanSwapchain* swapchain = nullptr;
    const VulkanMeshPass* meshPass = nullptr;
};

using DrawMeshBufferFn = std::function<void(VkCommandBuffer, const MeshBuffer&)>;

PreparedMeshPass BuildPreparedMeshPass(
    const chimi::renderer::RenderPacket& renderPacket,
    const VulkanMeshCache& meshCache);
void RecordPreparedMeshPassCommands(
    const MeshPassCommandContext& context,
    const PreparedMeshPass& preparedMeshPass,
    const DrawMeshBufferFn& drawMeshBuffer);
}
