/**
 * @file GraphicsPipelineBuilder.hpp
 * @brief Builder class for creating graphics pipelines in EasyVulkan framework
 * @details Provides a fluent interface for configuring and creating Vulkan graphics pipelines
 *          with sensible defaults and type-safe configuration.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <string>

namespace ev {

class VulkanDevice;
class VulkanContext;

/**
 * @class GraphicsPipelineBuilder
 * @brief Fluent builder interface for creating Vulkan graphics pipelines
 * @details GraphicsPipelineBuilder simplifies graphics pipeline creation by providing
 *          a fluent interface for configuring all pipeline states.
 *
 * Common usage patterns:
 * @code
 * // Create a basic graphics pipeline
 * auto pipeline = graphicsPipelineBuilder
 *     .setShaderStages(vertShader, fragShader)
 *     .setVertexInputState(bindingDescription, attributeDescriptions)
 *     .setViewport(viewport, scissor)
 *     .setRenderPass(renderPass, 0)
 *     .setDescriptorSetLayouts({descriptorSetLayout})
 *     .build("basicGraphics");
 *
 * // Create a pipeline with push constants
 * auto pipeline = graphicsPipelineBuilder
 *     .setShaderStages(vertShader, fragShader)
 *     .setVertexInputState(bindingDescription, attributeDescriptions)
 *     .setViewport(viewport, scissor)
 *     .setRenderPass(renderPass, 0)
 *     .setDescriptorSetLayouts({descriptorSetLayout})
 *     .addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
 *                          sizeof(PushConstants))
 *     .build("graphicsWithPushConstants");
 *
 * // Create a pipeline with multiple descriptor sets
 * auto pipeline = graphicsPipelineBuilder
 *     .setShaderStages(vertShader, fragShader)
 *     .setVertexInputState(bindingDescription, attributeDescriptions)
 *     .setViewport(viewport, scissor)
 *     .setRenderPass(renderPass, 0)
 *     .setDescriptorSetLayouts({globalLayout, materialLayout, objectLayout})
 *     .build("multiSetGraphics");
 * @endcode
 */
class GraphicsPipelineBuilder {
public:
    /**
     * @brief Constructor for GraphicsPipelineBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    explicit GraphicsPipelineBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Default destructor
     */
    ~GraphicsPipelineBuilder() = default;

    /**
     * @brief Adds a shader stage to the pipeline
     * @param stage Shader stage flag (e.g., VK_SHADER_STAGE_VERTEX_BIT)
     * @param module Shader module handle
     * @param entryPoint Name of the entry point function (default: "main")
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& addShaderStage(
        VkShaderStageFlagBits stage,
        VkShaderModule module,
        const char* entryPoint = "main");

    /**
     * @brief Sets the vertex input state
     * @param binding Vertex buffer binding description
     * @param attributes Vertex attribute descriptions
     * @return Reference to this builder for method chaining
     * @details Describes how vertex data is laid out in memory
     */
    GraphicsPipelineBuilder& setVertexInputState(
        const VkVertexInputBindingDescription& binding,
        const std::vector<VkVertexInputAttributeDescription>& attributes);


    /**
     * @brief Sets the input assembly state
     * @param topology Primitive topology (e.g., VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
     * @param primitiveRestart Whether to enable primitive restart
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setInputAssemblyState(
        VkPrimitiveTopology topology,
        VkBool32 primitiveRestart = VK_FALSE);

    /**
     * @brief Sets the viewport state
     * @param viewport Viewport description
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setViewport(const VkViewport& viewport);

    /**
     * @brief Sets the scissor rectangle
     * @param scissor Scissor rectangle description
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setScissor(const VkRect2D& scissor);

    /**
     * @brief Sets the rasterization state
     * @param polygonMode Fill mode for polygons
     * @param cullMode Face culling mode
     * @param frontFace Front face vertex order
     * @param lineWidth Width of rasterized lines
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setRasterizationState(
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL,
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT,
        VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE,
        float lineWidth = 1.0f);

    /**
     * @brief Sets the multisample state
     * @param samples Number of samples per pixel
     * @param sampleShading Enable/disable sample shading
     * @param minSampleShading Minimum sample shading factor
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setMultisampleState(
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
        VkBool32 sampleShading = VK_FALSE,
        float minSampleShading = 1.0f);

    /**
     * @brief Sets the depth/stencil state
     * @param depthTest Enable/disable depth testing
     * @param depthWrite Enable/disable depth writing
     * @param depthCompareOp Depth comparison function
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setDepthStencilState(
        VkBool32 depthTest = VK_TRUE,
        VkBool32 depthWrite = VK_TRUE,
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS);

    /**
     * @brief Sets the color blend state
     * @param attachments Color blend attachment states
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setColorBlendState(
        const std::vector<VkPipelineColorBlendAttachmentState>& attachments);

    /**
     * @brief Sets the dynamic state
     * @param dynamicStates List of states that can be dynamically changed
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setDynamicState(
        const std::vector<VkDynamicState>& dynamicStates);

    /**
     * @brief Sets the pipeline layout
     * @param layout Pipeline layout handle
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setLayout(VkPipelineLayout layout);

    /**
     * @brief Sets the render pass and subpass
     * @param renderPass Render pass handle
     * @param subpass Subpass index
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setRenderPass(
        VkRenderPass renderPass,
        uint32_t subpass = 0);

    /**
     * @brief Sets descriptor set layouts for the pipeline
     * @param setLayouts Vector of descriptor set layout handles
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& setDescriptorSetLayouts(
        const std::vector<VkDescriptorSetLayout>& setLayouts);

    /**
     * @brief Adds a push constant range to the pipeline
     * @param stageFlags Shader stages that can access the push constant
     * @param size Size of the push constant block in bytes
     * @param offset Offset into the push constant block (default: 0)
     * @return Reference to this builder for method chaining
     */
    GraphicsPipelineBuilder& addPushConstantRange(
        VkShaderStageFlags stageFlags,
        uint32_t size,
        uint32_t offset = 0);

    /**
     * @brief Builds and creates the graphics pipeline
     * @param name Optional debug name for the pipeline
     * @return Created pipeline handle
     * @throws std::runtime_error if pipeline creation fails or required states are missing
     */
    VkPipeline build(const std::string& name = "");

    /**
     * @brief Get the pipeline layout
     * @return The pipeline layout handle
     */
    VkPipelineLayout getPipelineLayout() const{
        return m_layout;
    }

private:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance

    VulkanContext* m_context;                ///< Pointer to VulkanContext instance

    // Pipeline state objects
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;  ///< Shader stages
    VkPipelineVertexInputStateCreateInfo m_vertexInputState{};    ///< Vertex input state
    VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyState{}; ///< Input assembly state
    VkPipelineViewportStateCreateInfo m_viewportState{};          ///< Viewport state
    VkPipelineRasterizationStateCreateInfo m_rasterizationState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    }; ///< Rasterization state
    VkPipelineMultisampleStateCreateInfo m_multisampleState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };    ///< Multisample state
    VkPipelineDepthStencilStateCreateInfo m_depthStencilState{};  ///< Depth/stencil state
    VkPipelineColorBlendStateCreateInfo m_colorBlendState{};      ///< Color blend state
    VkPipelineDynamicStateCreateInfo m_dynamicState{};            ///< Dynamic state

    // Pipeline configuration
    VkPipelineLayout m_layout{VK_NULL_HANDLE};  ///< Pipeline layout handle
    VkRenderPass m_renderPass{VK_NULL_HANDLE};  ///< Render pass handle
    uint32_t m_subpass{0};                      ///< Subpass index

    // Storage for dynamic arrays
    VkVertexInputBindingDescription m_vertexBinding;    ///< Vertex bindings
    std::vector<VkVertexInputAttributeDescription> m_vertexAttributes; ///< Vertex attributes
    VkViewport m_viewport{};                                          ///< Viewport state
    VkRect2D m_scissor{};                                            ///< Scissor rectangle
    std::vector<VkPipelineColorBlendAttachmentState> m_colorBlendAttachments; ///< Color blend states

    std::vector<VkDynamicState> m_dynamicStates;                     ///< Dynamic states

    std::vector<VkDescriptorSetLayout> m_setLayouts;      ///< Descriptor set layouts
    std::vector<VkPushConstantRange> m_pushConstantRanges; ///< Push constant ranges

    /**
     * @brief Initializes all pipeline states with sensible defaults
     */
    void initializeDefaults();

    /**
     * @brief Creates the pipeline layout from set layouts and push constants
     * @return Created pipeline layout handle
     * @throws std::runtime_error if layout creation fails
     */
    VkPipelineLayout createPipelineLayout();
};

} // namespace ev 