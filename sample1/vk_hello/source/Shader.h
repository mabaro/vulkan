#pragma once

#include "core/core.h"

#include <fstream>
#include <string>
#include <vector>

namespace utils {
////////////////////////////////////////////////////////////////////////////////

static std::vector<char>
readFile(const std::string& filename)
{
    std::vector<char> buffer;

    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Couldn't open the file '%s'", filename.c_str());
        return buffer;
    }

    const size_t fileSize = (size_t)file.tellg();
    buffer.resize(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

////////////////////////////////////////////////////////////////////////////////
}   // namespace utils

namespace gfx {
namespace vk {
////////////////////////////////////////////////////////////////////////////////

VkShaderModule
createShaderModule(VkDevice device, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        LOG_ERROR("Failed to create shader module");
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

////////////////////////////////////////////////////////////////////////////////
}   // namespace vk {
}   // namespace gfx
