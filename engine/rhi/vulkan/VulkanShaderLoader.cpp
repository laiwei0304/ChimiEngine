#include "rhi/vulkan/VulkanShaderLoader.h"

#include "core/Assert.h"

#include <fstream>
#include <stdexcept>
#include <string>

#include <volk.h>

namespace chimi::rhi::vulkan
{
std::vector<std::byte> ReadBinaryFile(const char* filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error(std::string("Failed to open file: ") + filePath);
    }

    const std::streamsize fileSize = file.tellg();
    std::vector<std::byte> buffer(static_cast<size_t>(fileSize));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    return buffer;
}

VkShaderModule CreateShaderModule(VkDevice device, const std::vector<std::byte>& shaderCode)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}
}
