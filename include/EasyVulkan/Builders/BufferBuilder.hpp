/**
 * @file BufferBuilder.hpp
 * @brief Builder class for creating Vulkan buffers in EasyVulkan framework
 * @details This file contains the BufferBuilder class which provides a fluent interface
 *          for creating and initializing Vulkan buffers with proper memory allocation.
 */

#pragma once

#include "../Common.hpp"

namespace ev {

class VulkanDevice;
class VulkanContext;

/**
 * @class BufferBuilder
 * @brief Fluent builder interface for creating Vulkan buffers
 * @details BufferBuilder simplifies buffer creation by:
 *          - Providing a fluent interface for configuration
 *          - Managing VMA (Vulkan Memory Allocator) allocation
 *          - Supporting automatic data upload
 *          - Providing sensible defaults for common use cases
 *
 * Common usage patterns:
 * @code
 * // Create a vertex buffer
 * auto vertexBuffer = resourceManager->createBuffer()
 *     .setSize(sizeof(vertices))
 *     .setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
 *     .setMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
 *     .setMemoryFlags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
 *                    VMA_ALLOCATION_CREATE_MAPPED_BIT)
 *     .build("myVertexBuffer");
 *
 * // Create and initialize a uniform buffer
 * UniformData data{...};
 * VmaAllocation allocation;
 * auto uniformBuffer = resourceManager->createBuffer()
 *     .setUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
 *     .setMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
 *     .setMemoryFlags(VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)
 *     .buildAndInitialize(&data, sizeof(data), "myUniformBuffer", &allocation);
 *
 * // Create a staging buffer for transfer operations
 * VmaAllocation stagingAllocation;
 * auto stagingBuffer = resourceManager->createBuffer()
 *     .setSize(dataSize)
 *     .setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
 *     .setMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
 *     .setMemoryFlags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
 *     .build("stagingBuffer", &stagingAllocation);
 * 
 * // Create a storage buffer used on GPU only
 * auto storageBuffer = resourceManager->createBuffer()
 *     .setSize(dataSize)
 *     .setUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
 *     .setMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
 *     .build("storageBuffer");
 * @endcode
 */
class BufferBuilder {
public:
    /**
     * @brief Constructor for BufferBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    explicit BufferBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Default destructor
     */
    ~BufferBuilder() = default;

    /**
     * @brief Sets the size of the buffer in bytes
     * @param size Size of the buffer in bytes
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if size is 0
     */
    BufferBuilder& setSize(VkDeviceSize size);

    /**
     * @brief Sets the usage flags for the buffer
     * @param usage Buffer usage flags
     * @return Reference to this builder for method chaining
     * 
     * Common usage flags:
     * - VK_BUFFER_USAGE_VERTEX_BUFFER_BIT: Buffer can be used as vertex buffer
     * - VK_BUFFER_USAGE_INDEX_BUFFER_BIT: Buffer can be used as index buffer
     * - VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT: Buffer can be used as uniform buffer
     * - VK_BUFFER_USAGE_STORAGE_BUFFER_BIT: Buffer can be used as storage buffer
     * - VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source for transfer
     * - VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination for transfer
     */
    BufferBuilder& setUsage(VkBufferUsageFlags usage);

    /**
     * @brief Sets the memory property flags for the buffer
     * @param properties Memory property flags
     * @return Reference to this builder for method chaining
     * 
     * Common property combinations:
     * - VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: GPU-only memory (fastest)
     * - VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
     *   CPU-accessible memory for frequent updates
     * - VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT:
     *   CPU-accessible GPU memory (if available)
     */
    BufferBuilder& setMemoryProperties(VkMemoryPropertyFlags properties);

    /**
     * @brief Sets the memory usage hint for VMA allocation
     * @param usage VMA memory usage hint
     * @return Reference to this builder for method chaining
     * 
     * Common usage values:
     * - VMA_MEMORY_USAGE_AUTO: Let VMA choose the best memory type
     * - VMA_MEMORY_USAGE_GPU_ONLY: GPU local memory for best performance
     * - VMA_MEMORY_USAGE_CPU_ONLY: CPU accessible memory for staging
     * - VMA_MEMORY_USAGE_CPU_TO_GPU: CPU visible memory for frequent updates
     * - VMA_MEMORY_USAGE_GPU_TO_CPU: GPU memory readable by CPU
     */
    BufferBuilder& setMemoryUsage(VmaMemoryUsage usage);

    /**
     * @brief Sets the allocation creation flags for VMA
     * @param flags VMA allocation creation flags
     * @return Reference to this builder for method chaining
     * 
     * Common flags:
     * - VMA_ALLOCATION_CREATE_MAPPED_BIT: Keep memory persistently mapped
     * - VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT: Optimize for sequential CPU writes
     * - VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT: Optimize for random CPU access
     * - VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT: Use dedicated memory allocation
     */
    BufferBuilder& setMemoryFlags(VmaAllocationCreateFlags flags);

    /**
     * @brief Sets the sharing mode for the buffer
     * @param sharingMode Sharing mode (exclusive or concurrent)
     * @return Reference to this builder for method chaining
     * 
     * @note Use VK_SHARING_MODE_CONCURRENT when buffer needs to be accessed
     *       by multiple queue families without explicit ownership transfers
     */
    BufferBuilder& setSharingMode(VkSharingMode sharingMode);

    /**
     * @brief Sets the queue family indices for concurrent sharing mode
     * @param queueFamilyIndices Vector of queue family indices
     * @return Reference to this builder for method chaining
     * 
     * @note Only required when using VK_SHARING_MODE_CONCURRENT
     */
    BufferBuilder& setQueueFamilyIndices(
        const std::vector<uint32_t>& queueFamilyIndices);

    /**
     * @brief Builds the buffer with current configuration
     * @param name Optional name for resource tracking
     * @param outAllocation Optional pointer to receive VMA allocation handle
     * @return Created buffer handle
     * @throws std::runtime_error if:
     *         - Buffer creation fails
     *         - Memory allocation fails
     *         - Required parameters are not set
     * 
     * Example:
     * @code
     * VmaAllocation allocation;
     * auto buffer = bufferBuilder
     *     ->setSize(size)
     *     ->setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
     *     ->build("myBuffer", &allocation);
     * @endcode
     */
    VkBuffer build(
        const std::string& name = "",
        VmaAllocation* outAllocation = nullptr);

    /**
     * @brief Builds the buffer and initializes it with data
     * @param data Pointer to the data to upload
     * @param dataSize Size of the data in bytes
     * @param name Optional name for resource tracking
     * @param outAllocation Optional pointer to receive VMA allocation handle
     * @return Created and initialized buffer handle
     * @throws std::runtime_error if:
     *         - Buffer creation fails
     *         - Memory allocation fails
     *         - Data upload fails
     *         - Data pointer is null
     *         - Data size doesn't match buffer size
     * 
     * Example:
     * @code
     * std::vector<Vertex> vertices = {...};
     * auto buffer = bufferBuilder
     *     ->setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
     *     ->buildAndInitialize(
     *         vertices.data(),
     *         vertices.size() * sizeof(Vertex),
     *         "vertexBuffer"
     *     );
     * @endcode
     */
    VkBuffer buildAndInitialize(
        const void* data,
        VkDeviceSize dataSize,
        const std::string& name = "",
        VmaAllocation* outAllocation = nullptr);

private:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance
    VulkanContext* m_context;                ///< Pointer to VulkanContext instance

    // Buffer creation parameters
    VkDeviceSize m_size{0};                  ///< Size of the buffer in bytes
    VkBufferUsageFlags m_usage{0};           ///< Buffer usage flags
    VmaMemoryUsage m_memoryUsage{VMA_MEMORY_USAGE_AUTO}; ///< Memory usage hint
    VmaAllocationCreateFlags m_memoryFlags{0}; ///< Allocation creation flags
    VkMemoryPropertyFlags m_memoryProperties{}; ///< Memory property flags
    VkSharingMode m_sharingMode{VK_SHARING_MODE_EXCLUSIVE}; ///< Buffer sharing mode
    std::vector<uint32_t> m_queueFamilyIndices; ///< Queue families for concurrent sharing

    /**
     * @brief Validates builder parameters before buffer creation
     * @throws std::runtime_error if parameters are invalid
     */
    void validateParameters() const;

    /**
     * @brief Creates the buffer using VMA
     * @param outAllocation Pointer to receive VMA allocation handle
     * @return Created buffer handle
     * @throws std::runtime_error if buffer creation fails
     */
    VkBuffer createBuffer(VmaAllocation* outAllocation) const;

    /**
     * @brief Uploads data to a buffer
     * @param buffer Buffer to upload to
     * @param allocation VMA allocation pointer for the buffer
     * @param data Pointer to data
     * @param dataSize Size of data in bytes
     */
    void uploadData(
        VkBuffer buffer,
        VmaAllocation* allocation,
        const void* data,
        VkDeviceSize dataSize) const;
};

} // namespace ev 