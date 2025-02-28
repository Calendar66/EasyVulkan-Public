/**
 * @file ComputePipelineBuilder.hpp
 * @brief Builder class for creating compute pipelines in EasyVulkan framework
 * @details This file contains the ComputePipelineBuilder class which provides a fluent interface
 *          for creating Vulkan compute pipelines with shader stages and pipeline layouts.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace ev {

class VulkanDevice;
class VulkanContext;

/**
 * @class ComputePipelineBuilder
 * @brief Fluent builder interface for creating Vulkan compute pipelines
 * @details ComputePipelineBuilder simplifies compute pipeline creation by:
 *          - Providing a fluent interface for configuration
 *          - Managing shader stages and pipeline layouts
 *          - Supporting pipeline derivatives
 *          - Providing sensible defaults for common use cases
 *          - Validating pipeline configuration
 *
 * Common usage patterns:
 * @code
 * // Create a basic compute pipeline
 * auto pipeline = computePipelineBuilder
 *     .setShaderStage(computeShaderModule)
 *     .setDescriptorSetLayouts({descriptorSetLayout})
 *     .build("basicCompute");
 *
 * // Create a compute pipeline with push constants
 * auto pipeline = computePipelineBuilder
 *     .setShaderStage(computeShaderModule)
 *     .setDescriptorSetLayouts({descriptorSetLayout})
 *     .addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PushConstants))
 *     .build("computeWithPushConstants");
 *
 * // Create a compute pipeline with multiple descriptor sets
 * auto pipeline = computePipelineBuilder
 *     .setShaderStage(computeShaderModule)
 *     .setDescriptorSetLayouts({globalLayout, materialLayout, objectLayout})
 *     .addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PushConstants))
 *     .build("multiSetCompute");
 *
 * // Create a derivative pipeline
 * auto derivedPipeline = computePipelineBuilder
 *     .setShaderStage(derivedShaderModule)
 *     .setDescriptorSetLayouts({descriptorSetLayout})
 *     .setBasePipeline(basePipeline)
 *     .build("derivedCompute");
 * @endcode
 */
class ComputePipelineBuilder {
public:
    /**
     * @brief Constructor for ComputePipelineBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    explicit ComputePipelineBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Virtual destructor for proper cleanup
     */
    ~ComputePipelineBuilder() = default;

    /**
     * @brief Sets the compute shader stage
     * @param module Shader module containing the compute shader
     * @param entryPoint Name of the entry point function (default: "main")
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Shader module is invalid
     *         - Entry point is null
     *
     * @note The shader module must contain a compute shader
     *       The entry point must exist in the shader module
     */
    ComputePipelineBuilder& setShaderStage(
        VkShaderModule module,
        const char* entryPoint = "main");

    /**
     * @brief Sets the pipeline layout
     * @param layout Pipeline layout handle
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if layout handle is invalid
     *
     * @note The pipeline layout defines the descriptor sets and push constants
     *       that can be accessed by the compute shader
     */
    ComputePipelineBuilder& setLayout(VkPipelineLayout layout);

    /**
     * @brief Sets the base pipeline for pipeline derivatives
     * @param basePipeline Handle to the parent pipeline
     * @param basePipelineIndex Index of the base pipeline (-1 to use handle)
     * @return Reference to this builder for method chaining
     *
     * @note Pipeline derivatives can reuse cache data from the parent pipeline
     *       This can improve pipeline creation performance
     *       Either basePipeline or basePipelineIndex should be used, not both
     */
    ComputePipelineBuilder& setBasePipeline(
        VkPipeline basePipeline,
        int32_t basePipelineIndex = -1);

    /**
     * @brief Sets descriptor set layouts for the pipeline
     * @param setLayouts Vector of descriptor set layout handles
     * @return Reference to this builder for method chaining
     */
    ComputePipelineBuilder& setDescriptorSetLayouts(
        const std::vector<VkDescriptorSetLayout>& setLayouts);

    /**
     * @brief Adds a push constant range to the pipeline
     * @param stageFlags Shader stages that can access the push constant
     * @param size Size of the push constant block in bytes
     * @param offset Offset into the push constant block (default: 0)
     * @return Reference to this builder for method chaining
     */
    ComputePipelineBuilder& addPushConstantRange(
        VkShaderStageFlags stageFlags,
        uint32_t size,
        uint32_t offset = 0);

    /**
     * @brief Builds the compute pipeline with current configuration
     * @param name Optional name for resource tracking
     * @return Created pipeline handle
     * @throws std::runtime_error if:
     *         - Shader stage is not set
     *         - Pipeline layout is not set
     *         - Pipeline creation fails
     *         - Base pipeline configuration is invalid
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

    // Pipeline state
    VkPipelineShaderStageCreateInfo m_shaderStage{}; ///< Compute shader stage info
    VkPipelineLayout m_layout{VK_NULL_HANDLE}; ///< Pipeline layout handle
    VkPipeline m_basePipeline{VK_NULL_HANDLE}; ///< Base pipeline for derivatives
    int32_t m_basePipelineIndex{-1};         ///< Base pipeline index

    std::vector<VkDescriptorSetLayout> m_setLayouts;      ///< Descriptor set layouts
    std::vector<VkPushConstantRange> m_pushConstantRanges; ///< Push constant ranges

    /**
     * @brief Validates the current builder state
     * @throws std::runtime_error if:
     *         - Required parameters are not set
     *         - Parameter combinations are invalid
     */
    void validateParameters() const;

    /**
     * @brief Creates the pipeline layout from set layouts and push constants
     * @return Created pipeline layout handle
     * @throws std::runtime_error if layout creation fails
     */
    VkPipelineLayout createPipelineLayout();
};

} // namespace ev 