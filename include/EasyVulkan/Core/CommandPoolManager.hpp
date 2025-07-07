/**
 * @file CommandPoolManager.hpp
 * @brief Command pool and buffer management class for EasyVulkan framework
 * @details This file contains the CommandPoolManager class which handles the creation,
 *          management, and cleanup of Vulkan command pools and command buffers.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>
#include <memory>

namespace ev {

class VulkanDevice;
class ResourceManager;

/**
 * @class CommandPoolManager
 * @brief Manages Vulkan command pools and command buffer allocation
 * @details CommandPoolManager provides:
 *          - Command pool creation and management per queue family
 *          - Command buffer allocation and lifecycle management
 *          - Utilities for single-time command execution
 *          - Automatic cleanup of command pools and buffers
 *
 * Common usage patterns:
 * @code
 * // Create a command pool for graphics queue
 * VkCommandPool graphicsPool = commandPoolManager->createCommandPool(
 *     device->getGraphicsQueueFamily(),
 *     VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
 * );
 *
 * // Allocate command buffers
 * auto cmdBuffers = commandPoolManager->allocateCommandBuffers(
 *     graphicsPool,
 *     VK_COMMAND_BUFFER_LEVEL_PRIMARY,
 *     swapchainImageCount
 * );
 *
 * // Execute single-time commands
 * auto cmdBuffer = commandPoolManager->beginSingleTimeCommands();
 * {
 *     // Record commands...
 * }
 * commandPoolManager->endSingleTimeCommands(cmdBuffer);
 * @endcode
 *
 * @note Inheritance:
 *       - Override single-time command methods for custom synchronization
 *       - Override pool creation for specialized memory management
 *       - Override command buffer allocation for custom pooling strategies
 */
class CommandPoolManager {
public:
    /**
     * @brief Constructor for CommandPoolManager
     * @param device Pointer to VulkanDevice instance
     * @throws std::runtime_error if device is nullptr
     */
    explicit CommandPoolManager(VulkanDevice* device);
    
    /**
     * @brief Virtual destructor for proper cleanup
     * @details Automatically cleans up all command pools and buffers
     */
    virtual ~CommandPoolManager();

    /**
     * @brief Creates a command pool for the specified queue family
     * @param queueFamilyIndex Queue family index to create pool for
     * @param flags Command pool creation flags
     * @return Created command pool handle
     * @throws std::runtime_error if:
     *         - Pool creation fails
     *         - Invalid queue family index
     * 
     * Common flags:
     * - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded frequently
     * - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be reset individually
     * 
     * Example:
     * @code
     * // Create a pool for graphics commands with reset capability
     * auto graphicsPool = commandPoolManager->createCommandPool(
     *     graphicsQueueFamily,
     *     VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
     * );
     * 
     * // Create a pool for transfer commands optimized for frequent rerecording
     * auto transferPool = commandPoolManager->createCommandPool(
     *     transferQueueFamily,
     *     VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
     * );
     * @endcode
     */
    virtual VkCommandPool createCommandPool(
        uint32_t queueFamilyIndex,
        VkCommandPoolCreateFlags flags = 0);

    /**
     * @brief Allocates command buffers from a pool
     * @param pool Command pool to allocate from
     * @param level Command buffer level (primary/secondary)
     * @param count Number of command buffers to allocate
     * @return Vector of allocated command buffer handles
     * @throws std::runtime_error if:
     *         - Allocation fails
     *         - Pool handle is invalid
     *         - Count is 0
     * 
     * Example:
     * @code
     * // Allocate primary command buffers for each swapchain image
     * auto cmdBuffers = commandPoolManager->allocateCommandBuffers(
     *     graphicsPool,
     *     VK_COMMAND_BUFFER_LEVEL_PRIMARY,
     *     swapchainImageCount
     * );
     * 
     * // Allocate secondary command buffers for multi-threaded recording
     * auto secondaryCmdBuffers = commandPoolManager->allocateCommandBuffers(
     *     graphicsPool,
     *     VK_COMMAND_BUFFER_LEVEL_SECONDARY,
     *     threadCount
     * );
     * @endcode
     */
    virtual std::vector<VkCommandBuffer> allocateCommandBuffers(
        VkCommandPool pool,
        VkCommandBufferLevel level,
        uint32_t count);

    /**
     * @brief Begins recording a single-time command buffer
     * @return Command buffer ready for recording
     * @throws std::runtime_error if:
     *         - Command buffer allocation fails
     *         - Recording cannot be started
     * 
     * Example:
     * @code
     * // Copy buffer data
     * auto cmdBuffer = commandPoolManager->beginSingleTimeCommands();
     * {
     *     VkBufferCopy copyRegion{};
     *     copyRegion.size = size;
     *     vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
     * }
     * commandPoolManager->endSingleTimeCommands(cmdBuffer);
     * @endcode
     */
    virtual VkCommandBuffer beginSingleTimeCommands();

    /**
     * @brief Ends and submits a single-time command buffer
     * @param commandBuffer Command buffer to submit
     * @throws std::runtime_error if:
     *         - Command buffer submission fails
     *         - Command buffer is invalid
     * 
     * @note This function waits for the command buffer to complete execution
     *       before returning. For asynchronous execution, use separate command
     *       pools and submission queues.
     */
    virtual void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    /**
     * @brief Frees command buffers
     * @param pool Command pool the buffers were allocated from
     * @param commandBuffers Vector of command buffers to free
     * @throws std::runtime_error if pool handle is invalid
     * 
     * Example:
     * @code
     * // Free command buffers when no longer needed
     * commandPoolManager->freeCommandBuffers(pool, {cmdBuffer1, cmdBuffer2});
     * @endcode
     */
    virtual void freeCommandBuffers(
        VkCommandPool pool,
        const std::vector<VkCommandBuffer>& commandBuffers);

    /**
     * @brief Resets a command pool
     * @param pool Command pool to reset
     * @param flags Reset flags
     * @throws std::runtime_error if:
     *         - Pool handle is invalid
     *         - Reset operation fails
     * 
     * Common flags:
     * - VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT: Release all resources owned by the pool
     * 
     * Example:
     * @code
     * // Reset pool at the start of each frame
     * commandPoolManager->resetCommandPool(
     *     graphicsPool,
     *     VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT
     * );
     * @endcode
     */
    virtual void resetCommandPool(
        VkCommandPool pool,
        VkCommandPoolResetFlags flags = 0);

    /**
     * @brief Returns the single-time command pool
     * @return Command pool for single-time commands
     */
    VkCommandPool getSingleTimeCommandPool() const { return m_singleTimeCommandPool; }

    /**
     * @brief Clears all allocated command buffers for a specified command pool
     * @param pool Command pool to clear buffers from
     * @param resourceManager ResourceManager instance to update tracked resources
     * @throws std::runtime_error if:
     *         - Pool handle is invalid
     *         - ResourceManager is nullptr
     * 
     * Example:
     * @code
     * // Clear all command buffers allocated from a specific pool
     * commandPoolManager->clearCommandBuffers(graphicsPool, resourceManager);
     * @endcode
     * 
     * @note This method:
     *       - Frees all command buffers allocated from the specified pool
     *       - Removes the freed command buffers from ResourceManager tracking
     *       - Does not destroy the command pool itself
     */
    virtual void clearCommandBuffers(
        VkCommandPool pool,
        ResourceManager* resourceManager);

protected:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance

    VkCommandPool m_singleTimeCommandPool;   ///< Pool for single-time commands

private:
    /// Map of queue family indices to their command pools
    std::unordered_map<uint32_t, std::vector<VkCommandPool>> m_commandPools;

    /**
     * @brief Cleans up all command pools
     * @details Called by destructor to ensure proper resource cleanup
     */
    void cleanup();

    /**
     * @brief Creates the command pool for single-time commands
     * @throws std::runtime_error if pool creation fails
     */
    void createSingleTimeCommandPool();
};

} // namespace ev 