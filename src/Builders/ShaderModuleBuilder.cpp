#include "EasyVulkan/Builders/ShaderModuleBuilder.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"
#include <fstream>
#include <stdexcept>

namespace ev {

ShaderModuleBuilder::ShaderModuleBuilder(VulkanDevice* device, VulkanContext* context)
    : m_device(device), m_context(context) {
}

ShaderModuleBuilder& ShaderModuleBuilder::setCode(
    const std::vector<uint32_t>& code) {
    
    m_code = code;
    return *this;
}

ShaderModuleBuilder& ShaderModuleBuilder::setCode(
    const void* pCode,
    size_t codeSize) {
    
    if (codeSize % sizeof(uint32_t) != 0) {
        throw std::runtime_error("Shader code size must be a multiple of 4");
    }

    const uint32_t* codePtr = reinterpret_cast<const uint32_t*>(pCode);
    m_code.assign(codePtr, codePtr + codeSize / sizeof(uint32_t));
    return *this;
}

ShaderModuleBuilder& ShaderModuleBuilder::loadFromFile(
    const std::string& filename) {
    
    m_code = loadSPIRVFromFile(filename);
    return *this;
}

void ShaderModuleBuilder::validateParameters() const {
    if (m_code.empty()) {
        throw std::runtime_error("No shader code provided");
    }
}

std::vector<uint32_t> ShaderModuleBuilder::loadSPIRVFromFile(
    const std::string& filename) const {
    
    // Open the file
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open shader file: " + filename);
    }

    // Get file size and validate
    size_t fileSize = static_cast<size_t>(file.tellg());
    if (fileSize % sizeof(uint32_t) != 0) {
        throw std::runtime_error("Shader file size must be a multiple of 4");
    }

    // Read the file
    std::vector<uint32_t> code(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(code.data()), fileSize);
    file.close();

    return code;
}

VkShaderModule ShaderModuleBuilder::build(const std::string& name) {
    validateParameters();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = m_code.size() * sizeof(uint32_t);
    createInfo.pCode = m_code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device->getLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    // Register the shader module for resource tracking if a name is provided
    if (!name.empty()) {
        auto *resourceManager = m_context->getResourceManager();
        resourceManager->registerResource(name,
                                        reinterpret_cast<uint64_t>(shaderModule),
                                        VK_OBJECT_TYPE_SHADER_MODULE);
    }

    return shaderModule;
}

} // namespace ev 