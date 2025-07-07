/**
 * @file CommandUtils.hpp
 * @brief Utility functions for Vulkan command buffer management and recording
 * @details This file provides a comprehensive set of utilities for:
 *          - Command buffer lifecycle management
 *          - Common Vulkan command recording
 *          - Resource state transitions
 *          - Memory barriers and synchronization
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace ev {

class VulkanDevice;

/**
 * @namespace CommandUtils
 * @brief Utilities for managing and recording Vulkan commands
 * @details Provides a simplified interface for common Vulkan command operations
 *          with built-in error checking and RAII patterns.
 *
 * Common usage patterns:
 * @code
 * // Single-time command execution
 * auto cmdBuffer = CommandUtils::beginSingleTimeCommands(device, pool);
 * {
 *     // Record commands...
 *     CommandUtils::bindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
 *     CommandUtils::bindVertexBuffers(cmdBuffer, 0, {vertexBuffer}, {0});
 *     CommandUtils::draw(cmdBuffer, vertexCount, 1, 0, 0);
 * }
 * CommandUtils::endSingleTimeCommands(device, pool, cmdBuffer);
 *
 * // Resource transitions
 * CommandUtils::transitionImageLayout(
 *     device, cmdBuffer, image,
 *     VK_IMAGE_LAYOUT_UNDEFINED,
 *     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
 *     {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
 * );
 * @endcode
 */
namespace CommandUtils {

/**
 * @brief Creates and begins a single-use command buffer
 * @param device The Vulkan device to allocate the command buffer from
 * @param pool The command pool to allocate from
 * @return A command buffer ready for recording
 * @throws std::runtime_error if:
 *         - Command buffer allocation fails
 *         - Command buffer recording cannot be started
 *         - Device or pool handles are invalid
 * 
 * Example:
 * @code
 * auto cmdBuffer = CommandUtils::beginSingleTimeCommands(device, pool);
 * // Record commands...
 * CommandUtils::endSingleTimeCommands(device, pool, cmdBuffer);
 * @endcode
 */
VkCommandBuffer beginSingleTimeCommands(VulkanDevice* device, VkCommandPool pool);

/**
 * @brief Ends, submits, and cleans up a single-use command buffer
 * @param device The Vulkan device that owns the command buffer
 * @param pool The command pool the buffer was allocated from
 * @param commandBuffer The command buffer to submit and clean up
 * @throws std::runtime_error if:
 *         - Command buffer submission fails
 *         - Queue submission fails
 *         - Fence creation or wait fails
 *         - Device or pool handles are invalid
 * 
 * @note This function will wait for the command buffer to complete execution
 *       before returning. For asynchronous execution, use separate submit commands.
 */
void endSingleTimeCommands(VulkanDevice* device, VkCommandPool pool, VkCommandBuffer commandBuffer);

/**
 * @brief Validates command buffer state
 * @param commandBuffer The command buffer to validate
 * @throws std::runtime_error if:
 *         - Command buffer is VK_NULL_HANDLE
 *         - Command buffer is not in recording state
 */
void validateCommandBuffer(VkCommandBuffer commandBuffer);

// Command Buffer Lifecycle Management
/**
 * @brief Begins recording a command buffer
 * @param commandBuffer The command buffer to begin recording
 * @param flags Usage flags controlling command buffer behavior
 * @throws std::runtime_error if:
 *         - Command buffer is invalid
 *         - Recording cannot be started
 * 
 * Common usage flags:
 * - VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
 * - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
 * - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
 */
void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags);

/**
 * @brief Ends recording a command buffer
 * @param commandBuffer The command buffer to end recording
 * @throws std::runtime_error if:
 *         - Command buffer is invalid
 *         - Recording cannot be ended properly
 *         - Command buffer is not in recording state
 */
void endCommandBuffer(VkCommandBuffer commandBuffer);

// Pipeline and Resource Binding Commands
/**
 * @brief Binds a pipeline to the command buffer
 * @param commandBuffer The command buffer to record the command into
 * @param pipelineBindPoint Specifies the pipeline type (graphics/compute)
 * @param pipeline The pipeline to bind
 * @throws std::runtime_error if command buffer validation fails
 * 
 * Example:
 * @code
 * CommandUtils::bindPipeline(
 *     cmdBuffer,
 *     VK_PIPELINE_BIND_POINT_GRAPHICS,
 *     graphicsPipeline
 * );
 * @endcode
 */
void bindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);

/**
 * @brief Binds descriptor sets to the command buffer
 * @param commandBuffer The command buffer to record the command into
 * @param pipelineBindPoint Specifies the pipeline type (graphics/compute)
 * @param layout The pipeline layout compatible with the descriptor sets
 * @param firstSet The first descriptor set binding number
 * @param descriptorSets The descriptor sets to bind
 * @param dynamicOffsets Optional dynamic offsets for dynamic descriptors
 * @throws std::runtime_error if:
 *         - Command buffer validation fails
 *         - Number of dynamic offsets doesn't match descriptor requirements
 * 
 * Example:
 * @code
 * CommandUtils::bindDescriptorSets(
 *     cmdBuffer,
 *     VK_PIPELINE_BIND_POINT_GRAPHICS,
 *     pipelineLayout,
 *     0,  // first set
 *     {descriptorSet},  // sets to bind
 *     {0}  // dynamic offsets
 * );
 * @endcode
 */
void bindDescriptorSets(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t firstSet,
    const std::vector<VkDescriptorSet>& descriptorSets,
    const std::vector<uint32_t>& dynamicOffsets = {});

/**
 * @brief Binds vertex buffers to the command buffer
 * @param commandBuffer The command buffer to record the command into
 * @param firstBinding The first binding number to bind the buffers to
 * @param buffers The vertex buffers to bind
 * @param offsets The offsets within each buffer to start reading from
 * @throws std::runtime_error if:
 *         - Command buffer validation fails
 *         - Number of buffers doesn't match number of offsets
 *         - Any buffer handle is invalid
 * 
 * Example:
 * @code
 * CommandUtils::bindVertexBuffers(
 *     cmdBuffer,
 *     0,  // first binding
 *     {vertexBuffer, instanceBuffer},  // buffers
 *     {0, 0}  // offsets
 * );
 * @endcode
 */
void bindVertexBuffers(
    VkCommandBuffer commandBuffer,
    uint32_t firstBinding,
    const std::vector<VkBuffer>& buffers,
    const std::vector<VkDeviceSize>& offsets);

/**
 * @brief Binds an index buffer for indexed drawing operations
 * @param commandBuffer The command buffer to record the command into
 * @param buffer The index buffer to bind
 * @param offset The offset within the buffer to start reading indices from
 * @param indexType The type of indices (VK_INDEX_TYPE_UINT16 or VK_INDEX_TYPE_UINT32)
 * @throws std::runtime_error if:
 *         - Command buffer validation fails
 *         - Buffer handle is invalid
 * 
 * Example:
 * @code
 * CommandUtils::bindIndexBuffer(
 *     cmdBuffer,
 *     indexBuffer,
 *     0,  // offset
 *     VK_INDEX_TYPE_UINT32
 * );
 * @endcode
 */
void bindIndexBuffer(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkIndexType indexType);

// Draw Commands
/**
 * @brief Records a non-indexed draw command
 * @param commandBuffer The command buffer to record the command into
 * @param vertexCount Number of vertices to draw
 * @param instanceCount Number of instances (1 for non-instanced)
 * @param firstVertex Index of the first vertex
 * @param firstInstance Index of the first instance
 * @throws std::runtime_error if command buffer validation fails
 * 
 * Example:
 * @code
 * CommandUtils::draw(
 *     cmdBuffer,
 *     3,    // vertex count (triangle)
 *     1,    // instance count
 *     0,    // first vertex
 *     0     // first instance
 * );
 * @endcode
 */
void draw(
    VkCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance);

/**
 * @brief Records an indexed draw command
 * @param commandBuffer The command buffer to record the command into
 * @param indexCount Number of indices to draw
 * @param instanceCount Number of instances (1 for non-instanced)
 * @param firstIndex Offset into the index buffer
 * @param vertexOffset Value added to indices
 * @param firstInstance Index of the first instance
 * @throws std::runtime_error if command buffer validation fails
 * 
 * Example:
 * @code
 * CommandUtils::drawIndexed(
 *     cmdBuffer,
 *     6,    // index count (2 triangles)
 *     1,    // instance count
 *     0,    // first index
 *     0,    // vertex offset
 *     0     // first instance
 * );
 * @endcode
 */
void drawIndexed(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance);

// Render Pass Commands
/**
 * Begins a render pass instance.
 * 
 * @param commandBuffer The command buffer to record the command into
 * @param renderPassBegin Structure specifying render pass begin info
 * @param contents Specifies how the commands within the render pass will be provided
 */
void beginRenderPass(
    VkCommandBuffer commandBuffer,
    const VkRenderPassBeginInfo& renderPassBegin,
    VkSubpassContents contents);

/**
 * Ends the current render pass instance.
 * 
 * @param commandBuffer The command buffer to record the command into
 */
void endRenderPass(VkCommandBuffer commandBuffer);

// Viewport and Scissor Commands
/**
 * Sets the viewport state dynamically.
 * 
 * @param commandBuffer The command buffer to record the command into
 * @param firstViewport Index of first viewport whose state is updated
 * @param viewports Array of viewport states to set
 */
void setViewport(
    VkCommandBuffer commandBuffer,
    uint32_t firstViewport,
    const std::vector<VkViewport>& viewports);

/**
 * @brief Sets the scissor rectangles dynamically.
 * 
 * @param commandBuffer The command buffer to record the command into
 * @param firstScissor Index of first scissor whose state is updated
 * @param scissors Array of scissor rectangles to set
 */
void setScissor(
    VkCommandBuffer commandBuffer,
    uint32_t firstScissor,
    const std::vector<VkRect2D>& scissors);

/**
 * @brief Updates push constant values.
 * 
 * @param commandBuffer The command buffer to record the command into
 * @param layout Pipeline layout used to program the push constant updates
 * @param stageFlags Shader stages that will use the push constants
 * @param offset Offset into the push constant block
 * @param size Size of the push constant block to update
 * @param pValues Values to update the push constants with
 */
void pushConstants(
    VkCommandBuffer commandBuffer,
    VkPipelineLayout layout,
    VkShaderStageFlags stageFlags,
    uint32_t offset,
    uint32_t size,
    const void* pValues);

/**
 * @brief Inserts a memory dependency in the command buffer.
 * 
 * @param commandBuffer The command buffer to record the command into
 * @param srcStageMask Pipeline stages to wait on
 * @param dstStageMask Pipeline stages that will wait
 * @param dependencyFlags Additional dependency flags
 * @param memoryBarriers Optional memory barriers
 * @param bufferMemoryBarriers Optional buffer memory barriers
 * @param imageMemoryBarriers Optional image memory barriers
 */
void pipelineBarrier(
    VkCommandBuffer commandBuffer,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    const std::vector<VkMemoryBarrier>& memoryBarriers = {},
    const std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers = {},
    const std::vector<VkImageMemoryBarrier>& imageMemoryBarriers = {});


/* -------------------------------------------------------------------------- */
/*                                    Copy                                    */
/* -------------------------------------------------------------------------- */
/**
 * @brief Copies data between buffer regions.
 * 
 * @param device The Vulkan device that owns the buffers
 * @param commandBuffer The command buffer to record the command into
 * @param srcBuffer The source buffer to copy from
 * @param dstBuffer The destination buffer to copy to
 * @param size The number of bytes to copy
 * @param srcOffset Optional offset into the source buffer
 * @param dstOffset Optional offset into the destination buffer
 */
void copyBuffer(
    VulkanDevice* device,
    VkCommandBuffer commandBuffer,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size,
    VkDeviceSize srcOffset = 0,
    VkDeviceSize dstOffset = 0);

/**
 * @brief Copies data from a buffer into an image.
 * 
 * @param device The Vulkan device that owns the resources
 * @param commandBuffer The command buffer to record the command into
 * @param srcBuffer The source buffer containing the pixel data
 * @param dstImage The destination image
 * @param width Width of the image region to copy
 * @param height Height of the image region to copy
 * @param layers Number of array layers to copy (defaults to 1)
 */
void copyBufferToImage(
    VulkanDevice* device,
    VkCommandBuffer commandBuffer,
    VkBuffer srcBuffer,
    VkImage dstImage,
    uint32_t width,
    uint32_t height,
    uint32_t layers = 1);



/**
 * @brief Copies data from one image to another image with automatic layout transitions
 * @details Records a complete image copy operation including automatic layout transitions.
 *          The function performs the following operations:
 *          1. Transitions source image to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL (if needed)
 *          2. Transitions destination image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL (if needed)
 *          3. Performs the vkCmdCopyImage operation
 *          4. Transitions source image back to original layout (if needed)
 *          5. Transitions destination image back to original layout (if needed)
 *          
 *          The copy operation supports:
 *          - Copying between different image formats (with compatible formats)
 *          - Copying specific mip levels and array layers
 *          - Copying with different aspect masks for source and destination
 *          - Automatic layout management for seamless integration
 * 
 * @param device The Vulkan device that owns the images
 * @param commandBuffer The command buffer to record the copy command into
 * @param srcImage The source image to copy from
 * @param dstImage The destination image to copy to
 * @param srcImageLayout The current layout of the source image (will be preserved)
 * @param dstImageLayout The current layout of the destination image (will be preserved)
 * @param srcAspectMask The aspect mask for the source image (e.g., VK_IMAGE_ASPECT_COLOR_BIT)
 * @param dstAspectMask The aspect mask for the destination image (e.g., VK_IMAGE_ASPECT_COLOR_BIT)
 * @param width The width of the region to copy
 * @param height The height of the region to copy
 * @param depth The depth of the region to copy (defaults to 1 for 2D images)
 * @param baseMipLevel The base mip level to copy from/to (defaults to 0)
 * @param levelCount The number of mip levels to copy (defaults to 1)
 * @param baseArrayLayer The base array layer to copy from/to (defaults to 0)
 * @param layerCount The number of array layers to copy (defaults to 1)
 * 
 * @throws std::runtime_error if command buffer validation fails
 * 
 * @note - Both images must have compatible formats and the specified region must be within
 *         the bounds of both images.
 *       - The function handles all necessary layout transitions automatically, so images
 *         can be in any valid layout before and after the operation.
 *       - Pipeline barriers are inserted to ensure proper synchronization between
 *         layout transitions and the copy operation.
 *       - The original layouts are preserved after the copy operation completes.
 * 
 * Simple usage example:
 * @code
 * // Copy entire 2D image (layouts handled automatically)
 * CommandUtils::copyImage(
 *     device,
 *     cmdBuffer,
 *     srcImage,
 *     dstImage,
 *     VK_IMAGE_LAYOUT_GENERAL,  // Current source layout
 *     VK_IMAGE_LAYOUT_GENERAL,  // Current destination layout
 *     VK_IMAGE_ASPECT_COLOR_BIT,
 *     VK_IMAGE_ASPECT_COLOR_BIT,
 *     1920,  // width
 *     1080   // height
 * );
 * // Both images are back to VK_IMAGE_LAYOUT_GENERAL after completion
 * @endcode
 * 
 * Advanced usage example:
 * @code
 * // Copy from compute shader output to graphics input
 * CommandUtils::copyImage(
 *     device,
 *     cmdBuffer,
 *     computeOutputImage,
 *     graphicsInputImage,
 *     VK_IMAGE_LAYOUT_GENERAL,                    // Compute output layout
 *     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   // Graphics input layout
 *     VK_IMAGE_ASPECT_COLOR_BIT,
 *     VK_IMAGE_ASPECT_COLOR_BIT,
 *     512,  // width
 *     512,  // height
 *     64,   // depth
 *     2,    // base mip level
 *     1,    // level count
 *     0,    // base array layer
 *     4     // layer count
 * );
 * // Images retain their original layouts after copy
 * @endcode
 */
void copyImage(
    VulkanDevice* device,
    VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImage dstImage,
    VkImageLayout srcImageLayout,
    VkImageLayout dstImageLayout,
    VkImageAspectFlags srcAspectMask,
    VkImageAspectFlags dstAspectMask,
    uint32_t width,
    uint32_t height,
    uint32_t depth = 1,
    uint32_t baseMipLevel = 0,
    uint32_t levelCount = 1,
    uint32_t baseArrayLayer = 0,
    uint32_t layerCount = 1);

/* -------------------------------------------------------------------------- */
/*                                    Clear                                   */
/* -------------------------------------------------------------------------- */
/**
 * @brief Clears color regions of an image.
 * 
 * @param commandBuffer The command buffer to record the command into
 * @param image The image to clear
 * @param imageLayout The current layout of the image
 * @param color The color value to clear with
 * @param ranges The image subresource ranges to clear
 */
void clearColorImage(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout imageLayout,
    const VkClearColorValue& color,
    const std::vector<VkImageSubresourceRange>& ranges);

/**
 * @brief Clears depth/stencil regions of an image.
 * 
 * @param commandBuffer The command buffer to record the command into
 * @param image The image to clear
 * @param imageLayout The current layout of the image
 * @param depthStencil The depth/stencil values to clear with
 * @param ranges The image subresource ranges to clear
 */
void clearDepthStencilImage(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout imageLayout,
    const VkClearDepthStencilValue& depthStencil,
    const std::vector<VkImageSubresourceRange>& ranges);

/**
 * @brief Inserts a compute-to-compute image barrier into the command buffer
 * @details This function inserts a pipeline barrier to synchronize compute shader access to an image.
 *          It ensures memory dependencies between compute shader writes and subsequent reads.
 *          Common use cases include:
 *          - Synchronizing between compute passes in multi-pass algorithms
 *          - Ensuring compute shader writes are visible to subsequent compute shader reads
 *          - Managing image layout transitions between compute passes
 * 
 * @param cmdBuffer The command buffer to record the command into
 * @param image The image to transition
 * @param oldLayout The old layout of the image (defaults to VK_IMAGE_LAYOUT_GENERAL)
 * @param newLayout The new layout of the image (defaults to VK_IMAGE_LAYOUT_GENERAL)
 * @param srcAccess The source access mask (defaults to VK_ACCESS_SHADER_WRITE_BIT)
 * @param dstAccess The destination access mask (defaults to VK_ACCESS_SHADER_READ_BIT)
 * @param aspectMask The aspect mask (defaults to VK_IMAGE_ASPECT_COLOR_BIT)
 * @param baseMipLevel The base mip level (defaults to 0)
 * @param levelCount The number of mip levels (defaults to 1)
 * @param baseArrayLayer The base array layer (defaults to 0)
 * @param layerCount The number of array layers (defaults to 1)
 * @param srcQueueFamily The source queue family (defaults to VK_QUEUE_FAMILY_IGNORED)
 * @param dstQueueFamily The destination queue family (defaults to VK_QUEUE_FAMILY_IGNORED)
 *
 * Simple usage example:
 * @code
 * // Basic barrier between compute shader writes and reads
 * CommandUtils::computeToComputeImageBarrier(
 *     cmdBuffer,
 *     computeImage
 * );
 * @endcode
 *
 * Complex usage example:
 * @code
 * // Barrier for multi-layer image with custom access masks and queue ownership transfer
 * CommandUtils::computeToComputeImageBarrier(
 *     cmdBuffer,
 *     computeImage,
 *     VK_IMAGE_LAYOUT_GENERAL,
 *     VK_IMAGE_LAYOUT_GENERAL,
 *     VK_ACCESS_SHADER_WRITE_BIT,
 *     VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
 *     VK_IMAGE_ASPECT_COLOR_BIT,
 *     0,  // base mip level
 *     3,  // 3 mip levels
 *     0,  // base array layer
 *     6,  // 6 array layers
 *     computeQueueFamily,
 *     transferQueueFamily
 * );
 * @endcode
 */
void computeToComputeImageBarrier(
    VkCommandBuffer cmdBuffer,
    VkImage image,
    VkImageLayout oldLayout=VK_IMAGE_LAYOUT_GENERAL,
    VkImageLayout newLayout=VK_IMAGE_LAYOUT_GENERAL,
    VkAccessFlags srcAccess = VK_ACCESS_SHADER_WRITE_BIT,
    VkAccessFlags dstAccess = VK_ACCESS_SHADER_READ_BIT,
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    uint32_t baseMipLevel = 0,
    uint32_t levelCount = 1,
    uint32_t baseArrayLayer = 0,
    uint32_t layerCount = 1,
    uint32_t srcQueueFamily = VK_QUEUE_FAMILY_IGNORED,
    uint32_t dstQueueFamily = VK_QUEUE_FAMILY_IGNORED
);


/**
 * @brief Creates a pipeline barrier for transitioning an image from compute shader to graphics shader usage
 * 
 * @param cmdBuffer The command buffer to record the barrier into
 * @param image The image to transition
 * @param oldLayout The old layout of the image (defaults to VK_IMAGE_LAYOUT_GENERAL)
 * @param newLayout The new layout of the image (defaults to VK_IMAGE_LAYOUT_GENERAL)
 * @param srcAccess Source access mask (defaults to VK_ACCESS_SHADER_WRITE_BIT)
 * @param dstAccess Destination access mask (defaults to VK_ACCESS_SHADER_READ_BIT)
 * @param aspectMask The aspect mask for the image (defaults to VK_IMAGE_ASPECT_COLOR_BIT)
 * @param baseMipLevel The base mip level (defaults to 0)
 * @param levelCount The number of mip levels (defaults to 1)
 * @param baseArrayLayer The base array layer (defaults to 0)
 * @param layerCount The number of array layers (defaults to 1)
 * @param srcQueueFamily The source queue family (defaults to VK_QUEUE_FAMILY_IGNORED)
 * @param dstQueueFamily The destination queue family (defaults to VK_QUEUE_FAMILY_IGNORED)
 *
 * Simple usage example:
 * @code
 * // Basic barrier from compute shader write to fragment shader read
 * CommandUtils::computeToGfxImageBarrier(
 *     cmdBuffer,
 *     computeImage
 * );
 * @endcode
 */
void computeToGfxImageBarrier(
    VkCommandBuffer      cmdBuffer,
    VkImage              image,
    VkImageLayout        oldLayout=VK_IMAGE_LAYOUT_GENERAL,
    VkImageLayout        newLayout=VK_IMAGE_LAYOUT_GENERAL,
    VkAccessFlags        srcAccess=VK_ACCESS_SHADER_WRITE_BIT,
    VkAccessFlags        dstAccess=VK_ACCESS_SHADER_READ_BIT,
    VkImageAspectFlags   aspectMask=VK_IMAGE_ASPECT_COLOR_BIT,
    uint32_t             baseMipLevel=0,
    uint32_t             levelCount=1,
    uint32_t             baseArrayLayer=0,
    uint32_t             layerCount=1,
    uint32_t             srcQueueFamily=VK_QUEUE_FAMILY_IGNORED,
    uint32_t             dstQueueFamily=VK_QUEUE_FAMILY_IGNORED
);

/**
 * @brief Creates a pipeline barrier for transitioning an image from graphics shader to compute shader usage
 * 
 * @param cmdBuffer The command buffer to record the barrier into
 * @param image The image to transition
 * @param oldLayout The old layout of the image (defaults to VK_IMAGE_LAYOUT_GENERAL)
 * @param newLayout The new layout of the image (defaults to VK_IMAGE_LAYOUT_GENERAL)
 * @param srcAccess Source access mask (defaults to VK_ACCESS_SHADER_WRITE_BIT)
 * @param dstAccess Destination access mask (defaults to VK_ACCESS_SHADER_READ_BIT)
 * @param aspectMask The aspect mask for the image (defaults to VK_IMAGE_ASPECT_COLOR_BIT)
 * @param baseMipLevel The base mip level (defaults to 0)
 * @param levelCount The number of mip levels (defaults to 1)
 * @param baseArrayLayer The base array layer (defaults to 0)
 * @param layerCount The number of array layers (defaults to 1)
 * @param srcQueueFamily The source queue family (defaults to VK_QUEUE_FAMILY_IGNORED)
 * @param dstQueueFamily The destination queue family (defaults to VK_QUEUE_FAMILY_IGNORED)
 *
 * Simple usage example:
 * @code
 * // Basic barrier from fragment shader write to compute shader read
 * CommandUtils::gfxToComputeImageBarrier(
 *     cmdBuffer,
 *     renderImage
 * );
 * @endcode
 */
void gfxToComputeImageBarrier(
    VkCommandBuffer      cmdBuffer,
    VkImage              image,
    VkImageLayout        oldLayout=VK_IMAGE_LAYOUT_GENERAL,
    VkImageLayout        newLayout=VK_IMAGE_LAYOUT_GENERAL,
    VkAccessFlags        srcAccess=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    VkAccessFlags        dstAccess=VK_ACCESS_SHADER_READ_BIT,
    VkImageAspectFlags   aspectMask=VK_IMAGE_ASPECT_COLOR_BIT,
    uint32_t             baseMipLevel=0,
    uint32_t             levelCount=1,
    uint32_t             baseArrayLayer=0,
    uint32_t             layerCount=1,
    uint32_t             srcQueueFamily=VK_QUEUE_FAMILY_IGNORED,
    uint32_t             dstQueueFamily=VK_QUEUE_FAMILY_IGNORED 
);

/**
 * @brief Creates a pipeline barrier for transitioning an image between graphics pipeline stages
 * 
 * @param cmdBuffer The command buffer to record the barrier into
 * @param image The image to transition
 * @param oldLayout The old layout of the image (defaults to VK_IMAGE_LAYOUT_GENERAL)
 * @param newLayout The new layout of the image (defaults to VK_IMAGE_LAYOUT_GENERAL)
 * @param srcAccess Source access mask (defaults to VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
 * @param dstAccess Destination access mask (defaults to VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
 * @param aspectMask The aspect mask for the image (defaults to VK_IMAGE_ASPECT_COLOR_BIT)
 * @param baseMipLevel The base mip level (defaults to 0)
 * @param levelCount The number of mip levels (defaults to 1)
 * @param baseArrayLayer The base array layer (defaults to 0)
 * @param layerCount The number of array layers (defaults to 1)
 * @param srcQueueFamily The source queue family (defaults to VK_QUEUE_FAMILY_IGNORED)
 * @param dstQueueFamily The destination queue family (defaults to VK_QUEUE_FAMILY_IGNORED)
 *
 * Simple usage example:
 * @code
 * // Barrier between fragment shader writes and subsequent fragment shader reads
 * CommandUtils::gfxToGfxImageBarrier(
 *     cmdBuffer,
 *     renderImage
 * );
 * 
 * // Barrier for subpass dependencies
 * CommandUtils::gfxToGfxImageBarrier(
 *     cmdBuffer,
 *     renderImage,
 *     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
 *     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
 * );
 * @endcode
 */
void gfxToGfxImageBarrier(
    VkCommandBuffer      cmdBuffer,
    VkImage              image,
    VkImageLayout        oldLayout=VK_IMAGE_LAYOUT_GENERAL,
    VkImageLayout        newLayout=VK_IMAGE_LAYOUT_GENERAL,
    VkAccessFlags        srcAccess=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    VkAccessFlags        dstAccess=VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
    VkImageAspectFlags   aspectMask=VK_IMAGE_ASPECT_COLOR_BIT,
    uint32_t             baseMipLevel=0,
    uint32_t             levelCount=1,
    uint32_t             baseArrayLayer=0,
    uint32_t             layerCount=1,
    uint32_t             srcQueueFamily=VK_QUEUE_FAMILY_IGNORED,
    uint32_t             dstQueueFamily=VK_QUEUE_FAMILY_IGNORED
);


} // namespace CommandUtils
} // namespace ev 