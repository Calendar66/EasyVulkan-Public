#include "EasyVulkan/Builders/GraphicsPipelineBuilder.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"
#include <stdexcept>

namespace ev {

GraphicsPipelineBuilder::GraphicsPipelineBuilder(VulkanDevice* device, VulkanContext* context)
    : m_device(device), m_context(context) {
    initializeDefaults();
}

void GraphicsPipelineBuilder::initializeDefaults() {
    // Initialize vertex input state
    m_vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_vertexInputState.vertexBindingDescriptionCount = 0;
    m_vertexInputState.pVertexBindingDescriptions = nullptr;
    m_vertexInputState.vertexAttributeDescriptionCount = 0;
    m_vertexInputState.pVertexAttributeDescriptions = nullptr;

    // Initialize input assembly state
    m_inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    // Initialize viewport state
    m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_viewportState.viewportCount = 1;
    m_viewportState.pViewports = &m_viewport;
    m_viewportState.scissorCount = 1;
    m_viewportState.pScissors = &m_scissor;

    // Initialize rasterization state
    m_rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_rasterizationState.depthClampEnable = VK_FALSE;
    m_rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    m_rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    m_rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    m_rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    m_rasterizationState.depthBiasEnable = VK_FALSE;
    m_rasterizationState.depthBiasConstantFactor = 0.0f;
    m_rasterizationState.depthBiasClamp = 0.0f;
    m_rasterizationState.depthBiasSlopeFactor = 0.0f;
    m_rasterizationState.lineWidth = 1.0f;

    // Initialize multisample state
    m_multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    m_multisampleState.sampleShadingEnable = VK_FALSE;
    m_multisampleState.minSampleShading = 1.0f;
    m_multisampleState.pSampleMask = nullptr;
    m_multisampleState.alphaToCoverageEnable = VK_FALSE;
    m_multisampleState.alphaToOneEnable = VK_FALSE;

    // Initialize depth stencil state
    m_depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_depthStencilState.depthTestEnable = VK_TRUE;
    m_depthStencilState.depthWriteEnable = VK_TRUE;
    m_depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
    m_depthStencilState.depthBoundsTestEnable = VK_FALSE;
    m_depthStencilState.stencilTestEnable = VK_FALSE;

    // Initialize color blend state
    m_colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_colorBlendState.logicOpEnable = VK_FALSE;
    m_colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    m_colorBlendState.attachmentCount = 0;
    m_colorBlendState.pAttachments = nullptr;
    m_colorBlendState.blendConstants[0] = 0.0f;
    m_colorBlendState.blendConstants[1] = 0.0f;
    m_colorBlendState.blendConstants[2] = 0.0f;
    m_colorBlendState.blendConstants[3] = 0.0f;

    // Initialize dynamic state
    m_dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    m_dynamicState.dynamicStateCount = 0;
    m_dynamicState.pDynamicStates = nullptr;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::addShaderStage(
    VkShaderStageFlagBits stage,
    VkShaderModule module,
    const char* entryPoint) {
    
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = stage;
    shaderStageInfo.module = module;
    shaderStageInfo.pName = entryPoint;

    m_shaderStages.push_back(shaderStageInfo);
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setVertexInputState(
    const VkVertexInputBindingDescription& binding,
    const std::vector<VkVertexInputAttributeDescription>& attributes) {
    

    m_vertexBinding = binding;
    m_vertexAttributes = attributes;


    m_vertexInputState.vertexBindingDescriptionCount = 1;
    m_vertexInputState.pVertexBindingDescriptions = &m_vertexBinding;
    m_vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    m_vertexInputState.pVertexAttributeDescriptions = m_vertexAttributes.data();


    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setInputAssemblyState(
    VkPrimitiveTopology topology,
    VkBool32 primitiveRestart) {
    
    m_inputAssemblyState.topology = topology;
    m_inputAssemblyState.primitiveRestartEnable = primitiveRestart;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setViewport(const VkViewport& viewport) {
    m_viewport = viewport;
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setScissor(const VkRect2D& scissor) {
    m_scissor = scissor;
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setRasterizationState(
    VkPolygonMode polygonMode,
    VkCullModeFlags cullMode,
    VkFrontFace frontFace,
    float lineWidth) {
    
    m_rasterizationState.polygonMode = polygonMode;
    m_rasterizationState.cullMode = cullMode;
    m_rasterizationState.frontFace = frontFace;
    m_rasterizationState.lineWidth = lineWidth;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setMultisampleState(
    VkSampleCountFlagBits samples,
    VkBool32 sampleShading,
    float minSampleShading) {
    
    m_multisampleState.rasterizationSamples = samples;
    m_multisampleState.sampleShadingEnable = sampleShading;
    m_multisampleState.minSampleShading = minSampleShading;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDepthStencilState(
    VkBool32 depthTest,
    VkBool32 depthWrite,
    VkCompareOp depthCompareOp) {
    
    m_depthStencilState.depthTestEnable = depthTest;
    m_depthStencilState.depthWriteEnable = depthWrite;
    m_depthStencilState.depthCompareOp = depthCompareOp;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setColorBlendState(
    const std::vector<VkPipelineColorBlendAttachmentState>& attachments) {
    
    m_colorBlendAttachments = attachments;
    m_colorBlendState.attachmentCount = static_cast<uint32_t>(attachments.size());
    m_colorBlendState.pAttachments = m_colorBlendAttachments.data();

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDynamicState(
    const std::vector<VkDynamicState>& dynamicStates) {
    
    m_dynamicStates = dynamicStates;
    m_dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    m_dynamicState.pDynamicStates = m_dynamicStates.data();

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setLayout(VkPipelineLayout layout) {
    m_layout = layout;
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setRenderPass(
    VkRenderPass renderPass,
    uint32_t subpass) {
    
    m_renderPass = renderPass;
    m_subpass = subpass;
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDescriptorSetLayouts(
    const std::vector<VkDescriptorSetLayout>& setLayouts) {
    m_setLayouts = setLayouts;
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::addPushConstantRange(
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

VkPipelineLayout GraphicsPipelineBuilder::createPipelineLayout() {
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

VkPipeline GraphicsPipelineBuilder::build(const std::string& name) {
    if (m_shaderStages.empty()) {
        throw std::runtime_error("No shader stages specified for graphics pipeline");
    }

    if (m_layout == VK_NULL_HANDLE) {
        m_layout = createPipelineLayout();
    }

    if (m_renderPass == VK_NULL_HANDLE) {
        throw std::runtime_error("Render pass not specified");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
    pipelineInfo.pStages = m_shaderStages.data();
    pipelineInfo.pVertexInputState = &m_vertexInputState;
    pipelineInfo.pInputAssemblyState = &m_inputAssemblyState;
    pipelineInfo.pViewportState = &m_viewportState;
    pipelineInfo.pRasterizationState = &m_rasterizationState;
    pipelineInfo.pMultisampleState = &m_multisampleState;
    pipelineInfo.pDepthStencilState = &m_depthStencilState;
    pipelineInfo.pColorBlendState = &m_colorBlendState;
    pipelineInfo.pDynamicState = m_dynamicState.dynamicStateCount > 0 ? &m_dynamicState : nullptr;
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = m_subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(m_device->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // Register the pipeline for resource tracking if a name is provided
    if (!name.empty()) {
        auto *resourceManager = m_context->getResourceManager();
        resourceManager->registerResource(name,
                                        reinterpret_cast<uint64_t>(pipeline),
                                        reinterpret_cast<uint64_t>(m_layout),
                                        VK_OBJECT_TYPE_PIPELINE);
    }

    return pipeline;
}

} // namespace ev 