/**
 * @file ResourceUtils.hpp
 * @brief Utility functions for resource management in EasyVulkan framework
 * @details This file contains utilities for creating and managing Vulkan resources:
 *          - Buffers with automatic memory allocation
 *          - Images and image views
 *          - Shader modules
 *          - Resource loading helpers
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "EasyVulkan/Core/VulkanDevice.hpp"

namespace ev {

class VulkanDevice;

/**
 * @namespace ResourceUtils
 * @brief Namespace containing Vulkan resource management utilities
 * @details Provides functionality for:
 *          - Creating buffers with automatic memory allocation
 *          - Creating images with proper memory allocation
 *          - Creating image views with common configurations
 *          - Loading and creating shader modules
 *          - Managing resource lifecycle
 *
 * Common usage patterns:
 * @code
 * // Create a vertex buffer
 * VmaAllocation allocation;
 * auto vertexBuffer = ResourceUtils::createBuffer(
 *     device,
 *     sizeof(vertices),
 *     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
 *     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
 *     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
 *     &allocation
 * );
 *
 * // Create a texture image
 * auto textureImage = ResourceUtils::createImage(
 *     device,
 *     width,
 *     height,
 *     VK_FORMAT_R8G8B8A8_SRGB,
 *     VK_IMAGE_TILING_OPTIMAL,
 *     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
 *     VK_IMAGE_USAGE_SAMPLED_BIT,
 *     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
 * );
 *
 * // Load and create a shader module
 * auto shaderCode = ResourceUtils::loadShaderCode("shader.spv");
 * auto shaderModule = ResourceUtils::createShaderModule(
 *     device,
 *     shaderCode
 * );
 * @endcode
 */
namespace ResourceUtils {

/**
 * @brief Creates a buffer with automatic memory allocation
 * @param device Pointer to VulkanDevice instance
 * @param size Size of the buffer in bytes
 * @param usage Buffer usage flags
 * @param properties Memory property flags
 * @param outAllocation Optional pointer to receive VMA allocation handle
 * @return Created buffer handle
 * @throws std::runtime_error if:
 *         - Buffer creation fails
 *         - Memory allocation fails
 *         - Size is 0
 *
 * Common usage combinations:
 * - Vertex buffer:
 *   @code
 *   createBuffer(
 *       device,
 *       vertexSize,
 *       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
 *       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
 *       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
 *   );
 *   @endcode
 *
 * - Index buffer:
 *   @code
 *   createBuffer(
 *       device,
 *       indexSize,
 *       VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
 *       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
 *       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
 *   );
 *   @endcode
 *
 * - Uniform buffer:
 *   @code
 *   createBuffer(
 *       device,
 *       sizeof(UniformData),
 *       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
 *       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
 *       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
 *   );
 *   @endcode
 *
 * - Staging buffer:
 *   @code
 *   createBuffer(
 *       device,
 *       dataSize,
 *       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
 *       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
 *       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
 *   );
 *   @endcode
 */
VkBuffer createBuffer(
    VulkanDevice* device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VmaAllocation* outAllocation = nullptr);

/**
 * @brief Creates an image with automatic memory allocation
 * @param device Pointer to VulkanDevice instance
 * @param width Width of the image in pixels
 * @param height Height of the image in pixels
 * @param format Image format
 * @param tiling Image tiling mode
 * @param usage Image usage flags
 * @param properties Memory property flags
 * @param outAllocation Optional pointer to receive VMA allocation handle
 * @return Created image handle
 * @throws std::runtime_error if:
 *         - Image creation fails
 *         - Memory allocation fails
 *         - Invalid dimensions
 *
 * Common usage combinations:
 * - Texture image:
 *   @code
 *   createImage(
 *       device,
 *       width,
 *       height,
 *       VK_FORMAT_R8G8B8A8_SRGB,
 *       VK_IMAGE_TILING_OPTIMAL,
 *       VK_IMAGE_USAGE_TRANSFER_DST_BIT |
 *       VK_IMAGE_USAGE_SAMPLED_BIT,
 *       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
 *   );
 *   @endcode
 *
 * - Depth buffer:
 *   @code
 *   createImage(
 *       device,
 *       width,
 *       height,
 *       VK_FORMAT_D32_SFLOAT,
 *       VK_IMAGE_TILING_OPTIMAL,
 *       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
 *       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
 *   );
 *   @endcode
 *
 * - Color attachment:
 *   @code
 *   createImage(
 *       device,
 *       width,
 *       height,
 *       swapchainFormat,
 *       VK_IMAGE_TILING_OPTIMAL,
 *       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
 *       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
 *   );
 *   @endcode
 */
VkImage createImage(
    VulkanDevice* device,
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VmaAllocation* outAllocation = nullptr);

/**
 * @brief Creates an image view for an image
 * @param device Pointer to VulkanDevice instance
 * @param image Image handle to create view for
 * @param format Format of the image
 * @param aspectFlags Image aspect flags
 * @return Created image view handle
 * @throws std::runtime_error if:
 *         - Image view creation fails
 *         - Image handle is invalid
 *
 * Common aspect flags:
 * - Color image: VK_IMAGE_ASPECT_COLOR_BIT
 * - Depth image: VK_IMAGE_ASPECT_DEPTH_BIT
 * - Depth+Stencil: VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
 *
 * Example:
 * @code
 * // Create view for color texture
 * auto textureView = createImageView(
 *     device,
 *     textureImage,
 *     VK_FORMAT_R8G8B8A8_SRGB,
 *     VK_IMAGE_ASPECT_COLOR_BIT
 * );
 *
 * // Create view for depth buffer
 * auto depthView = createImageView(
 *     device,
 *     depthImage,
 *     VK_FORMAT_D32_SFLOAT,
 *     VK_IMAGE_ASPECT_DEPTH_BIT
 * );
 * @endcode
 */
VkImageView createImageView(
    VulkanDevice* device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags);

/**
 * @brief Creates a shader module from SPIR-V code
 * @param device Pointer to VulkanDevice instance
 * @param code Vector containing SPIR-V bytecode
 * @return Created shader module handle
 * @throws std::runtime_error if:
 *         - Shader module creation fails
 *         - Code vector is empty
 *         - Code size is not a multiple of 4
 *
 * Example:
 * @code
 * // Load and create vertex shader
 * auto vertCode = loadShaderCode("vert.spv");
 * auto vertModule = createShaderModule(device, vertCode);
 *
 * // Load and create fragment shader
 * auto fragCode = loadShaderCode("frag.spv");
 * auto fragModule = createShaderModule(device, fragCode);
 * @endcode
 */
VkShaderModule createShaderModule(
    VulkanDevice* device,
    const std::vector<uint32_t>& code);

/**
 * @brief Loads SPIR-V shader code from a file
 * @param filename Path to the SPIR-V shader file
 * @return Vector containing the SPIR-V bytecode
 * @throws std::runtime_error if:
 *         - File cannot be opened
 *         - File is empty
 *         - File size is not a multiple of 4
 *
 * Example:
 * @code
 * // Load vertex shader
 * auto vertCode = loadShaderCode("shaders/vert.spv");
 *
 * // Load fragment shader
 * auto fragCode = loadShaderCode("shaders/frag.spv");
 *
 * // Load compute shader
 * auto compCode = loadShaderCode("shaders/comp.spv");
 * @endcode
 */
std::vector<uint32_t> loadShaderCode(const std::string& filename);


/**
 * @brief Uploads data to an image
 * @param device Pointer to VulkanDevice instance
 * @param commandPool Command pool to allocate temporary command buffer from
 * @param image Image to upload data to
 * @param data Pointer to the image data
 * @param dataSize Size of the data in bytes
 * @param width Width of the image
 * @param height Height of the image
 * @throws std::runtime_error if:
 *         - Data pointer is null
 *         - Data size is 0
 *         - Staging buffer creation fails
 *         - Memory allocation fails
 *         - Image layout transition fails
 *
 * Example:
 * @code
 * // Create and upload a texture image
 * std::vector<uint8_t> imageData = loadImageData("texture.png");
 * VkImage textureImage = createImage(device, width, height, format);
 * 
 * uploadDataToImage(
 *     device,
 *     commandPool,
 *     textureImage,
 *     imageData.data(),
 *     imageData.size(),
 *     width,
 *     height
 * );
 * @endcode
 */
void uploadDataToImage(
    VulkanDevice* device, 
    VkCommandPool commandPool, 
    VkImage image, 
    const void* data, 
    VkDeviceSize dataSize, 
    uint32_t width, 
    uint32_t height);

/**
 * @brief Uploads data to a buffer at a specific offset
 * @param buffer Buffer to upload to
 * @param device Pointer to VulkanDevice instance
 * @param allocation VMA allocation pointer for the buffer
 * @param data Pointer to data
 * @param dataSize Size of data in bytes
 * @param offset Offset in bytes from the start of the buffer
 * 
 * Example:
 * @code
 * // Upload vertex data to a buffer
 * std::vector<Vertex> vertices = {...};
 * uploadDataToBuffer(
 *     vertexBuffer,
 *     device,
 *     &allocation,
 *     vertices.data(),
 *     vertices.size() * sizeof(Vertex),
 *     0
 * );
 * @endcode
 */
void uploadDataToBuffer(
    VkBuffer buffer,
    VulkanDevice* device,
    VmaAllocation* allocation,
    const void* data,
    VkDeviceSize dataSize,
    VkDeviceSize offset); 

/**
 * @brief Uploads data to a buffer(Mapped) at a specific offset
 * @param buffer Buffer to upload to
 * @param device Pointer to VulkanDevice instance
 * @param allocation VMA allocation pointer for the buffer
 * @param data Pointer to data
 * @param dataSize Size of data in bytes
 * @param offset Offset in bytes from the start of the buffer
 */
void uploadDataToMappedBuffer(
    VkBuffer buffer,
    VulkanDevice* device,
    VmaAllocation* allocation,
    const void* data,
    VkDeviceSize dataSize,
    VkDeviceSize offset);


/**
 * @brief Transitions an image's layout using a temporary command buffer
 * @param device Pointer to VulkanDevice instance
 * @param commandPool Command pool to allocate temporary command buffer from
 * @param image Image to transition
 * @param oldLayout Current layout
 * @param newLayout Desired layout
 * @throws std::runtime_error if:
 *         - Layout transition fails
 *         - Invalid layout combination
 * 
 * Similar to transitionImageLayout() but creates and manages its own temporary command buffer.
 * Useful when no existing command buffer is available.
 * 
 * Example:
 * @code
 * transitionImageLayoutWithoutCommandBuffer(
 *     device,
 *     commandPool,
 *     image,
 *     VK_IMAGE_LAYOUT_UNDEFINED,
 *     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
 * );
 * @endcode
 */
void transitionImageLayoutWithoutCommandBuffer(VulkanDevice* device, VkCommandPool commandPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

/**
 * @brief Transitions an image's layout using an existing command buffer
 * 
 * @param device Pointer to VulkanDevice instance
 * @param commandBuffer Command buffer to record transition commands into
 * @param image Image to transition
 * @param oldLayout Current layout
 * @param newLayout Desired layout
 * @throws std::runtime_error if:
 *         - Layout transition fails
 *         - Invalid layout combination
 * 
 * Common transitions:
 * - For texture upload:
 *   @code
 *   transitionImageLayout(
 *       device,
 *       commandBuffer,
 *       image,
 *       VK_IMAGE_LAYOUT_UNDEFINED,
 *       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
 *   );
 *   @endcode
 * 
 * - After upload:
 *   @code
 *   transitionImageLayout(
 *       device,
 *       commandBuffer, 
 *       image,
 *       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
 *       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
 *   );
 *   @endcode
 * 
 * - For compute shader access:
 *   @code
 *   transitionImageLayout(
 *       device,
 *       commandBuffer,
 *       image, 
 *       VK_IMAGE_LAYOUT_UNDEFINED,
 *       VK_IMAGE_LAYOUT_GENERAL
 *   );
 *   @endcode
 */
void transitionImageLayout(
    VulkanDevice* device,
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout);

/**
 * @brief Transitions an image's layout using ImageInfo struct
 * 
 * @param device The Vulkan device that owns the image
 * @param commandBuffer Command buffer to record the transition command
 * @param imageInfo Structure containing image handle and current layout
 * @param newLayout Desired layout to transition to
 * @throws std::runtime_error if:
 *         - Layout transition fails
 *         - Invalid layout combination
 * 
 * This is a convenience wrapper around transitionImageLayout() that uses
 * ImageInfo struct to track the current layout and automatically skip
 * unnecessary transitions.
 * 
 * Example:
 * @code
 * // Transition image to shader read layout
 * transitionImageLayoutWithInfo(
 *     device,
 *     commandBuffer,
 *     imageInfo,
 *     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
 * );
 * @endcode
 */
void transitionImageLayoutWithInfo(
    VulkanDevice* device,
    VkCommandPool commandPool,
    ImageInfo &imageInfo,
    VkImageLayout newLayout);



} // namespace ResourceUtils
} // namespace ev 