#include "rhi/vulkan/VulkanMeshPipeline.h"

#include "core/Assert.h"
#include "renderer/MeshData.h"
#include "rhi/vulkan/VulkanShaderLoader.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <stdexcept>

#include <volk.h>

namespace chimi::rhi::vulkan
{
namespace
{
struct MeshDrawPushConstants
{
    glm::mat4 objectToClip{ 1.0f };
    glm::vec4 lightDirectionAndIntensity{ -0.6f, -1.0f, -0.35f, 1.0f };
    glm::vec4 lightColorAndAmbient{ 1.0f, 0.98f, 0.92f, 0.18f };
};
}

void SyncMeshPassPipeline(
    VulkanMeshPass& meshPass,
    VkDevice device,
    VkFormat colorFormat,
    VkFormat depthFormat,
    const chimi::renderer::MeshPassPipelineConfig& config)
{
    if (meshPass.pipelineState.valid && meshPass.pipelineState.config.pipelineKind == config.pipelineKind
        && meshPass.pipelineState.config.cullMode == config.cullMode
        && meshPass.pipelineState.config.frontFace == config.frontFace
        && meshPass.pipelineState.config.depthTestEnabled == config.depthTestEnabled
        && meshPass.pipelineState.config.depthWriteEnabled == config.depthWriteEnabled)
    {
        return;
    }

    DestroyMeshPass(meshPass, device);

    const std::filesystem::path shaderDirectory = CHIMI_SHADER_OUTPUT_DIR;
    const char* vertexShaderName = nullptr;
    const char* fragmentShaderName = nullptr;

    switch (config.pipelineKind)
    {
    case chimi::renderer::MeshPassPipelineKind::SolidColor:
        vertexShaderName = "mesh.vert.spv";
        fragmentShaderName = "mesh.frag.spv";
        break;
    default:
        throw std::runtime_error("Unsupported mesh pipeline kind");
    }

    const std::vector<std::byte> vertexShaderCode =
        ReadBinaryFile((shaderDirectory / vertexShaderName).string().c_str());
    const std::vector<std::byte> fragmentShaderCode =
        ReadBinaryFile((shaderDirectory / fragmentShaderName).string().c_str());

    const VkShaderModule vertexShaderModule = CreateShaderModule(device, vertexShaderCode);
    const VkShaderModule fragmentShaderModule = CreateShaderModule(device, fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";

    const VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertexShaderStageInfo,
        fragmentShaderStageInfo
    };

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(chimi::renderer::Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(chimi::renderer::Vertex, position);
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(chimi::renderer::Vertex, color);
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(chimi::renderer::Vertex, normal);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    switch (config.cullMode)
    {
    case chimi::renderer::MeshCullMode::None:
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        break;
    case chimi::renderer::MeshCullMode::Back:
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        break;
    default:
        throw std::runtime_error("Unsupported mesh cull mode");
    }

    switch (config.frontFace)
    {
    case chimi::renderer::MeshFrontFace::CounterClockwise:
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        break;
    case chimi::renderer::MeshFrontFace::Clockwise:
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        break;
    default:
        throw std::runtime_error("Unsupported mesh front face");
    }

    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = config.depthTestEnabled ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = config.depthWriteEnabled ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    const VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(MeshDrawPushConstants);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &meshPass.pipelineLayout));

    VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingInfo.colorAttachmentCount = 1;
    pipelineRenderingInfo.pColorAttachmentFormats = &colorFormat;
    pipelineRenderingInfo.depthAttachmentFormat = depthFormat;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &pipelineRenderingInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = meshPass.pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = 0;

    VK_CHECK(vkCreateGraphicsPipelines(
        device,
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        nullptr,
        &meshPass.graphicsPipeline));

    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(device, vertexShaderModule, nullptr);

    meshPass.pipelineState.config = config;
    meshPass.pipelineState.valid = true;
}

void DestroyMeshPass(VulkanMeshPass& meshPass, VkDevice device)
{
    if (meshPass.graphicsPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, meshPass.graphicsPipeline, nullptr);
        meshPass.graphicsPipeline = VK_NULL_HANDLE;
    }

    if (meshPass.pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, meshPass.pipelineLayout, nullptr);
        meshPass.pipelineLayout = VK_NULL_HANDLE;
    }

    meshPass.pipelineState.valid = false;
}
}
