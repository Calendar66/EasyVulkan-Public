#include "EasyVulkan/Builders/ComputePipelineBuilder.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"
#include <stdexcept>

namespace ev {

ComputePipelineBuilder::ComputePipelineBuilder(VulkanDevice* device, VulkanContext* context)
    : m_device(device) , m_context(context) {
    // Initialize shader stage create info
    m_shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    m_shaderStage.pName = "main";
}

ComputePipelineBuilder& ComputePipelineBuilder::setShaderStage(
    VkShaderModule module,
    const char* entryPoint) {
    m_shaderStage.module = module;
    m_shaderStage.pName = entryPoint;
    return *this;
}

ComputePipelineBuilder& ComputePipelineBuilder::setLayout(VkPipelineLayout layout) {
    m_layout = layout;
    return *this;
}

ComputePipelineBuilder& ComputePipelineBuilder::setBasePipeline(
    VkPipeline basePipeline,
    int32_t basePipelineIndex) {
    m_basePipeline = basePipeline;
    m_basePipelineIndex = basePipelineIndex;
    return *this;
}

ComputePipelineBuilder& ComputePipelineBuilder::setDescriptorSetLayouts(
    const std::vector<VkDescriptorSetLayout>& setLayouts) {
    m_setLayouts = setLayouts;
    return *this;
}

ComputePipelineBuilder& ComputePipelineBuilder::addPushConstantRange(
    VkShaderStageFlags stageFlags,
    uint32_t size,
    uint32_t offset) {
    VkPushConstantRange range{};
    range.stageFlags = stageFlags;
    range.size = size;
    range.offset = offset;
    m_pushConstantRanges.push_back(range);
    return *this;
}

void ComputePipelineBuilder::validateParameters() const {
    if (!m_device) {
        throw std::runtime_error("No device provided to ComputePipelineBuilder");
    }
    if (m_shaderStage.module == VK_NULL_HANDLE) {
        throw std::runtime_error("No shader module provided to ComputePipelineBuilder");
    }
    if (m_layout == VK_NULL_HANDLE) {
        throw std::runtime_error("No pipeline layout provided to ComputePipelineBuilder");
    }
}

VkPipelineLayout ComputePipelineBuilder::createPipelineLayout() {
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = static_cast<uint32_t>(m_setLayouts.size());
    layoutInfo.pSetLayouts = m_setLayouts.data();
    layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_pushConstantRanges.size());
    layoutInfo.pPushConstantRanges = m_pushConstantRanges.data();

    VkPipelineLayout layout;
    if (vkCreatePipelineLayout(m_device->getLogicalDevice(), &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }
    return layout;
}

VkPipeline ComputePipelineBuilder::build(const std::string& name) {
    // Create pipeline layout if not explicitly set
    if (m_layout == VK_NULL_HANDLE) {
        m_layout = createPipelineLayout();
    }

    validateParameters();

    VkComputePipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.stage = m_shaderStage;
    createInfo.layout = m_layout;
    createInfo.basePipelineHandle = m_basePipeline;
    createInfo.basePipelineIndex = m_basePipelineIndex;

    VkPipeline pipeline;
    VkResult result = vkCreateComputePipelines(
        m_device->getLogicalDevice(),
        VK_NULL_HANDLE,
        1,
        &createInfo,
        nullptr,
        &pipeline);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline");
    }

    // Register pipeline with resource manager if name is provided
    if (!name.empty()) {
        auto* resourceManager = m_context->getResourceManager();
        resourceManager->registerResource2(name, 
                                        reinterpret_cast<uint64_t>(pipeline),
                                        reinterpret_cast<uint64_t>(m_layout),
                                        VK_OBJECT_TYPE_PIPELINE);
    }


    return pipeline;
}

} // namespace ev 