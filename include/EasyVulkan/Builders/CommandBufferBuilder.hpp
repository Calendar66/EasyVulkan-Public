/**
 * @file CommandBufferBuilder.hpp
 * @brief Builder class for creating and configuring Vulkan command buffers
 * @details This file contains the CommandBufferBuilder class which provides a fluent
 *          interface for creating and configuring command buffers with proper setup
 *          and resource tracking.
 */

#pragma once

#include "../Common.hpp"
#include <vector>

namespace ev {

class VulkanDevice;
class VulkanContext;

/**
 * @class CommandBufferBuilder
 * @brief Fluent builder interface for creating Vulkan command buffers
 * @details CommandBufferBuilder simplifies command buffer creation by:
 *          - Providing a fluent interface for configuration
 *          - Managing command buffer allocation
 *          - Supporting both single and multiple command buffer creation
 *          - Handling resource tracking
 *          - Providing sensible defaults
 *
 * Common usage patterns:
 * @code
 * // Create a single primary command buffer
 * auto cmdBuffer = commandBufferBuilder
 *     ->setCommandPool(graphicsPool)
 *     ->setLevel(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
 *     ->build("mainCommandBuffer");
 *
 * // Create multiple command buffers for swapchain images
 * auto cmdBuffers = commandBufferBuilder
 *     ->setCommandPool(graphicsPool)
 *     ->setCount(swapchainImageCount)
 *     ->buildMultiple({"frame0", "frame1", "frame2"});
 *
 * // Create secondary command buffers for multi-threaded recording
 * auto secondaryCmdBuffers = commandBufferBuilder
 *     ->setCommandPool(graphicsPool)
 *     ->setLevel(VK_COMMAND_BUFFER_LEVEL_SECONDARY)
 *     ->setCount(threadCount)
 *     ->buildMultiple();
 * @endcode
 */
class CommandBufferBuilder {
public:
    /**
     * @brief Constructor for CommandBufferBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    CommandBufferBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Sets the command buffer level
     * @param level Command buffer level (primary or secondary)
     * @return Reference to this builder for method chaining
     *
     * Command buffer levels:
     * - VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to queues
     * - VK_COMMAND_BUFFER_LEVEL_SECONDARY: Can be executed by primary command buffers
     *
     * Example:
     * @code
     * // Create a primary command buffer
     * auto primaryCmd = builder
     *     ->setLevel(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
     *     ->build();
     *
     * // Create a secondary command buffer
     * auto secondaryCmd = builder
     *     ->setLevel(VK_COMMAND_BUFFER_LEVEL_SECONDARY)
     *     ->build();
     * @endcode
     */
    CommandBufferBuilder& setLevel(VkCommandBufferLevel level);

    /**
     * @brief Sets the command pool to allocate from
     * @param pool Command pool handle
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if pool handle is VK_NULL_HANDLE
     *
     * Example:
     * @code
     * // Allocate from graphics pool
     * auto cmdBuffer = builder
     *     ->setCommandPool(graphicsPool)
     *     ->build();
     *
     * // Allocate from compute pool
     * auto computeCmd = builder
     *     ->setCommandPool(computePool)
     *     ->build();
     * @endcode
     */
    CommandBufferBuilder& setCommandPool(VkCommandPool pool);

    /**
     * @brief Sets the command buffer usage flags
     * @param flags Command buffer usage flags
     * @return Reference to this builder for method chaining
     *
     * Common usage flags:
     * - VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: Buffer will be rerecorded after use
     * - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: Secondary buffer used inside render pass
     * - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: Buffer can be resubmitted while pending
     *
     * Example:
     * @code
     * // Create a one-time-submit buffer
     * auto cmdBuffer = builder
     *     ->setUsageFlags(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
     *     ->build();
     * @endcode
     */
    CommandBufferBuilder& setUsageFlags(VkCommandBufferUsageFlags flags);

    /**
     * @brief Sets the number of command buffers to allocate
     * @param count Number of command buffers to create
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if count is 0
     *
     * Example:
     * @code
     * // Create command buffers for each swapchain image
     * auto cmdBuffers = builder
     *     ->setCount(swapchainImageCount)
     *     ->buildMultiple();
     * @endcode
     */
    CommandBufferBuilder& setCount(uint32_t count);

    /**
     * @brief Builds and returns a single command buffer
     * @param name Optional name for resource tracking and debugging
     * @return Created command buffer handle
     * @throws std::runtime_error if:
     *         - Command pool is not set
     *         - Allocation fails
     *         - Count is not 1
     *
     * Example:
     * @code
     * auto cmdBuffer = builder
     *     ->setCommandPool(graphicsPool)
     *     ->build("mainCommandBuffer");
     * @endcode
     */
    VkCommandBuffer build(const std::string& name = "");

    /**
     * @brief Builds and returns multiple command buffers
     * @param names Optional vector of names for resource tracking and debugging
     * @return Vector of created command buffer handles
     * @throws std::runtime_error if:
     *         - Command pool is not set
     *         - Allocation fails
     *         - Names vector size doesn't match count (if names provided)
     *
     * Example:
     * @code
     * auto cmdBuffers = builder
     *     ->setCommandPool(graphicsPool)
     *     ->setCount(3)
     *     ->buildMultiple({"frame0", "frame1", "frame2"});
     * @endcode
     */
    std::vector<VkCommandBuffer> buildMultiple(const std::vector<std::string>& names = {});

private:
    /**
     * @brief Validates builder parameters before command buffer creation
     * @throws std::runtime_error if parameters are invalid
     */
    void validateParameters() const;

    /**
     * @brief Creates command buffers with current configuration
     * @param names Optional vector of names for resource tracking
     * @return Vector of created command buffer handles
     * @throws std::runtime_error if creation fails
     */
    std::vector<VkCommandBuffer> createCommandBuffers(const std::vector<std::string>& names = {}) const;

    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance
    VulkanContext* m_context;                ///< Pointer to VulkanContext instance
    VkCommandBufferLevel m_level{VK_COMMAND_BUFFER_LEVEL_PRIMARY}; ///< Command buffer level
    VkCommandPool m_commandPool{VK_NULL_HANDLE}; ///< Command pool for allocation
    VkCommandBufferUsageFlags m_usageFlags{0}; ///< Command buffer usage flags
    uint32_t m_count{1};                     ///< Number of command buffers to create
};

} // namespace ev
