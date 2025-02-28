/**
 * @file MemoryUtils.hpp
 * @brief Utility functions for memory management in EasyVulkan framework
 * @details This file contains utilities for managing Vulkan memory, including:
 *          - Memory type selection
 *          - Memory mapping and data transfer
 *          - Memory requirements querying
 *          - VMA (Vulkan Memory Allocator) integration
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace ev {

class VulkanDevice;

/**
 * @namespace MemoryUtils
 * @brief Namespace containing Vulkan memory management utilities
 * @details Provides functionality for:
 *          - Finding appropriate memory types
 *          - Mapping memory for CPU access
 *          - Copying data between CPU and GPU memory
 *          - Querying memory requirements
 *          - Managing VMA allocations
 *
 * Common usage patterns:
 * @code
 * // Find suitable memory type for a buffer
 * uint32_t memoryType = MemoryUtils::findMemoryType(
 *     device,
 *     memRequirements.memoryTypeBits,
 *     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
 *     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
 * );
 *
 * // Upload data to a buffer
 * MemoryUtils::mapAndCopyData(
 *     device,
 *     allocation,
 *     vertices.data(),
 *     sizeof(vertices[0]) * vertices.size()
 * );
 *
 * // Get memory requirements for resource allocation
 * auto memReqs = MemoryUtils::getBufferMemoryRequirements(
 *     device,
 *     buffer
 * );
 * @endcode
 */
namespace MemoryUtils {

/**
 * @brief Finds a memory type index that satisfies requirements
 * @param device Pointer to VulkanDevice instance
 * @param typeFilter Bit field of allowed memory types
 * @param properties Required memory property flags
 * @return Index of suitable memory type
 * @throws std::runtime_error if no suitable memory type is found
 *
 * Common property combinations:
 * - Device local (GPU) memory:
 *   @code
 *   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
 *   @endcode
 *
 * - Host visible coherent memory:
 *   @code
 *   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
 *   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
 *   @endcode
 *
 * - Host visible cached memory:
 *   @code
 *   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
 *   VK_MEMORY_PROPERTY_HOST_CACHED_BIT
 *   @endcode
 *
 * Example:
 * @code
 * // Find memory type for a staging buffer
 * uint32_t memoryTypeIndex = findMemoryType(
 *     device,
 *     memRequirements.memoryTypeBits,
 *     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
 *     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
 * );
 * @endcode
 */
uint32_t findMemoryType(
    VulkanDevice* device,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties);

/**
 * @brief Maps memory and copies data to it
 * @param device Pointer to VulkanDevice instance
 * @param allocation VMA allocation handle
 * @param data Pointer to source data
 * @param size Size of data in bytes
 * @throws std::runtime_error if:
 *         - Memory mapping fails
 *         - Data pointer is null
 *         - Size is 0
 *         - Memory copy fails
 *
 * Example:
 * @code
 * // Upload vertex data to a buffer
 * std::vector<Vertex> vertices = {...};
 * mapAndCopyData(
 *     device,
 *     bufferAllocation,
 *     vertices.data(),
 *     vertices.size() * sizeof(Vertex)
 * );
 *
 * // Upload uniform data
 * UniformBufferObject ubo = {...};
 * mapAndCopyData(
 *     device,
 *     uniformAllocation,
 *     &ubo,
 *     sizeof(UniformBufferObject)
 * );
 * @endcode
 */
void mapAndCopyData(
    VulkanDevice* device,
    VmaAllocation allocation,
    const void* data,
    VkDeviceSize size);

/**
 * @brief Maps memory and retrieves data from it
 * @param device Pointer to VulkanDevice instance
 * @param allocation VMA allocation handle
 * @param data Pointer to destination buffer
 * @param size Size of data in bytes
 * @throws std::runtime_error if:
 *         - Memory mapping fails
 *         - Data pointer is null
 *         - Size is 0
 *         - Memory copy fails
 *
 * Example:
 * @code
 * // Read back vertex data
 * std::vector<Vertex> vertices(vertexCount);
 * mapAndRetrieveData(
 *     device,
 *     bufferAllocation,
 *     vertices.data(),
 *     vertices.size() * sizeof(Vertex)
 * );
 *
 * // Read compute shader results
 * std::vector<float> results(resultCount);
 * mapAndRetrieveData(
 *     device,
 *     storageAllocation,
 *     results.data(),
 *     results.size() * sizeof(float)
 * );
 * @endcode
 */
void mapAndRetrieveData(
    VulkanDevice* device,
    VmaAllocation allocation,
    void* data,
    VkDeviceSize size);

/**
 * @brief Gets memory requirements for a buffer
 * @param device Pointer to VulkanDevice instance
 * @param buffer Buffer handle
 * @return Memory requirements for the buffer
 * @throws std::runtime_error if buffer handle is invalid
 *
 * Example:
 * @code
 * // Get memory requirements for buffer allocation
 * auto memReqs = getBufferMemoryRequirements(device, buffer);
 * VkMemoryAllocateInfo allocInfo{};
 * allocInfo.allocationSize = memReqs.size;
 * allocInfo.memoryTypeIndex = findMemoryType(
 *     device,
 *     memReqs.memoryTypeBits,
 *     properties
 * );
 * @endcode
 */
VkMemoryRequirements getBufferMemoryRequirements(
    VulkanDevice* device,
    VkBuffer buffer);

/**
 * @brief Gets memory requirements for an image
 * @param device Pointer to VulkanDevice instance
 * @param image Image handle
 * @return Memory requirements for the image
 * @throws std::runtime_error if image handle is invalid
 *
 * Example:
 * @code
 * // Get memory requirements for image allocation
 * auto memReqs = getImageMemoryRequirements(device, image);
 * VkMemoryAllocateInfo allocInfo{};
 * allocInfo.allocationSize = memReqs.size;
 * allocInfo.memoryTypeIndex = findMemoryType(
 *     device,
 *     memReqs.memoryTypeBits,
 *     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
 * );
 * @endcode
 */
VkMemoryRequirements getImageMemoryRequirements(
    VulkanDevice* device,
    VkImage image);

} // namespace MemoryUtils

} // namespace ev 