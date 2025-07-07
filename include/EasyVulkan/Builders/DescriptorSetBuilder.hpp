/**
 * @file DescriptorSetBuilder.hpp
 * @brief Builder class for creating descriptor sets in EasyVulkan framework
 * @details This file contains the DescriptorSetBuilder class which provides a fluent interface
 *          for creating Vulkan descriptor sets, layouts, and managing descriptor updates.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace ev {

class VulkanDevice;
class VulkanContext;

/**
 * @class DescriptorSetBuilder
 * @brief Fluent builder interface for creating Vulkan descriptor sets
 * @details DescriptorSetBuilder simplifies descriptor set creation by:
 *          - Providing a fluent interface for configuration
 *          - Managing descriptor set layouts
 *          - Handling descriptor updates
 *          - Supporting multiple descriptor types (uniform buffers, storage buffers, combined image samplers, storage images)
 *          - Managing descriptor pools
 *
 * Common usage patterns:
 * @code
 * // Create a descriptor set for a uniform buffer
 * auto descriptorSet = builder
 *     .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
 *     .addBufferDescriptor(0, uniformBuffer, 0, sizeof(UniformData))
 *     .buildWithLayout("myDescriptorSet");
 *
 * // Create a descriptor set for a storage image
 * auto descriptorSet = builder
 *     .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT)
 *     .addStorageImageDescriptor(0, imageView, VK_IMAGE_LAYOUT_GENERAL)
 *     .buildWithLayout("computeDescriptor");
 *
 * // Create a descriptor set with multiple storage images
 * auto descriptorSet = builder
 *     .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT)
 *     .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT) 
 *     .addStorageImageDescriptor(0, inputImageView, VK_IMAGE_LAYOUT_GENERAL)
 *     .addStorageImageDescriptor(1, outputImageView, VK_IMAGE_LAYOUT_GENERAL)
 *     .buildWithLayout("computeDescriptor");
 * @endcode
 */
class DescriptorSetBuilder {
public:
    /**
     * @brief Constructor for DescriptorSetBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    explicit DescriptorSetBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Virtual destructor for proper cleanup
     */
    ~DescriptorSetBuilder() = default;

    /**
     * @brief Adds a descriptor binding to the layout
     * @param binding Binding number in shader
     * @param type Type of descriptor (uniform buffer, storage buffer, etc.)
     * @param count Number of descriptors in array (1 for non-array)
     * @param stageFlags Shader stages that will access this descriptor
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if binding parameters are invalid
     *
     * Common descriptor types:
     * - VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: Uniform buffer
     * - VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: Storage buffer
     * - VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: Texture sampler
     * - VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: Storage image
     */
    DescriptorSetBuilder& addBinding(
        uint32_t binding,
        VkDescriptorType type,
        uint32_t count,
        VkShaderStageFlags stageFlags);

    /**
     * @brief Adds a buffer descriptor to the set
     * @param binding Binding number (must match a previous addBinding call)
     * @param buffer Buffer handle
     * @param offset Offset into the buffer
     * @param range Size of the buffer region to bind
     * @param type Type of buffer descriptor (default: uniform buffer)
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Binding not found
     *         - Buffer handle is invalid
     *         - Type doesn't match binding
     */
    DescriptorSetBuilder& addBufferDescriptor(
        uint32_t binding,
        VkBuffer buffer,
        VkDeviceSize offset,
        VkDeviceSize range,
        VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    /**
     * @brief Adds an image descriptor to the set
     * @param binding Binding number (must match a previous addBinding call)
     * @param imageView Image view handle
     * @param sampler Sampler handle
     * @param imageLayout Layout the image will be in when accessed
     * @param type Type of image descriptor (default: combined image sampler)
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Binding not found
     *         - Image view or sampler handle is invalid
     *         - Type doesn't match binding
     */
    DescriptorSetBuilder& addImageDescriptor(
        uint32_t binding,
        VkImageView imageView,
        VkSampler sampler,
        VkImageLayout imageLayout,
        VkDescriptorType type);

    /**
     * @brief Adds a storage image descriptor to the set
     * @param binding Binding number (must match a previous addBinding call)
     * @param imageView Image view handle
     * @param imageLayout Layout the image will be in when accessed
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Binding not found
     *         - Image view handle is invalid
     *         - Binding type is not storage image
     */
    DescriptorSetBuilder& addStorageImageDescriptor(
        uint32_t binding,
        VkImageView imageView,
        VkImageLayout imageLayout);

    /**
     * @brief Creates the descriptor set layout
     * @param name Optional name for resource tracking
     * @return Created descriptor set layout handle
     * @throws std::runtime_error if:
     *         - No bindings have been added
     *         - Layout creation fails
     */
    VkDescriptorSetLayout createLayout(const std::string& name = "");

    /**
     * @brief Builds the descriptor set using an existing layout
     * @param layout Descriptor set layout to use
     * @param name Optional name for resource tracking
     * @return Created descriptor set handle
     * @throws std::runtime_error if:
     *         - Layout is invalid
     *         - Descriptor pool creation fails
     *         - Descriptor set allocation fails
     *         - Descriptor updates fail
     */
    VkDescriptorSet build(
        VkDescriptorSetLayout layout,
        const std::string& name = "");

    /**
     * @brief Builds the descriptor set with a new layout
     * @param name Optional name for resource tracking
     * @return Created descriptor set handle
     * @throws std::runtime_error if:
     *         - Layout creation fails
     *         - Descriptor pool creation fails
     *         - Descriptor set allocation fails
     *         - Descriptor updates fail
     */
    VkDescriptorSet buildWithLayout(const std::string& name = "");

    /**
     * @brief Updates a descriptor set with current descriptors
     * @param descriptorSet Descriptor set to update
     * @throws std::runtime_error if update fails
     */
    void updateDescriptorSet(VkDescriptorSet descriptorSet);

private:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance
    VulkanContext* m_context;                ///< Pointer to VulkanContext instance

    // Layout bindings
    std::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;    ///< Descriptor set layout bindings

    // Write descriptors
    std::vector<VkWriteDescriptorSet> m_writes;                    ///< Descriptor write operations
    std::vector<bool> m_writeUpdated;                              ///< Track which writes have been updated
    // (NOTE:If we want to add more than 16 buffer descriptors, we need to change the size of the vector)
    std::vector<VkDescriptorBufferInfo> m_bufferInfos{32};            ///< Buffer descriptor info with pre-reserved memory
    unsigned int m_bufferInfoCount = 0;           ///< Number of buffer info
    std::vector<VkDescriptorImageInfo> m_imageInfos{32};          ///< Image descriptor info with pre-reserved memory
    unsigned int m_imageInfoCount = 0;           ///< Number of image info

    /**
     * @brief Validates binding configuration
     * @throws std::runtime_error if bindings are invalid
     */
    void validateBindings() const;

    /**
     * @brief Creates a descriptor pool for the current set of descriptors
     * @return Created descriptor pool handle
     * @throws std::runtime_error if pool creation fails
     */
    VkDescriptorPool createPool() const;
};

} // namespace ev 