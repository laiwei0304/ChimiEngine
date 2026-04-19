#pragma once

#include <cstddef>
#include <vector>

#include <vulkan/vulkan.h>

namespace chimi::rhi::vulkan
{
std::vector<std::byte> ReadBinaryFile(const char* filePath);
VkShaderModule CreateShaderModule(VkDevice device, const std::vector<std::byte>& shaderCode);
}
