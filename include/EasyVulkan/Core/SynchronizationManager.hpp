/**
 * @file SynchronizationManager.hpp
 * @brief Synchronization primitives management class for EasyVulkan framework
 * @details This file contains the SynchronizationManager class which handles creation
 *          and management of Vulkan synchronization primitives (semaphores and fences)
 *          for GPU-GPU and CPU-GPU synchronization.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

namespace ev {

class VulkanDevice;

/**
 * @class SynchronizationManager
 * @brief Manages Vulkan synchronization primitives
 * @details SynchronizationManager provides:
 *          - Creation and management of semaphores and fences
 *          - Per-frame synchronization primitives for swapchain rendering
 *          - Named tracking of synchronization objects
 *          - Automatic cleanup of resources
 *
 * Common usage patterns:
 * @code
 * // Create synchronization objects for frame rendering
 * syncManager->createFrameSynchronization(MAX_FRAMES_IN_FLIGHT);
 *
 * // In render loop:
 * auto imageAvailable = syncManager->getImageAvailableSemaphore(currentFrame);
 * auto renderFinished = syncManager->getRenderFinishedSemaphore(currentFrame);
 * auto inFlightFence = syncManager->getInFlightFence(currentFrame);
 *
 * // Wait for previous frame
 * syncManager->waitForFences({inFlightFence});
 * syncManager->resetFences({inFlightFence});
 *
 * // Acquire next image and render...
 * vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, ...);
 * // Submit command buffer with appropriate synchronization
 * submitInfo.waitSemaphores = {imageAvailable};
 * submitInfo.signalSemaphores = {renderFinished};
 * @endcode
 *
 * @note Inheritance:
 *       - Override creation methods for specialized synchronization strategies
 *       - Override wait methods for custom timeout handling
 *       - Override frame synchronization for different rendering patterns
 */
class SynchronizationManager {
public:
    /**
     * @brief Constructor for SynchronizationManager
     * @param device Pointer to VulkanDevice instance
     * @throws std::runtime_error if device is nullptr
     */
    explicit SynchronizationManager(VulkanDevice* device);
    
    /**
     * @brief Virtual destructor for proper cleanup
     * @details Automatically destroys all tracked semaphores and fences
     */
    virtual ~SynchronizationManager();

    /**
     * @brief Creates a binary semaphore
     * @param name Optional name for tracking and debugging
     * @return Created semaphore handle
     * @throws std::runtime_error if:
     *         - Semaphore creation fails
     *         - Device is invalid
     * 
     * Example:
     * @code
     * // Create named semaphores for custom synchronization
     * auto transferComplete = syncManager->createSemaphore("transferComplete");
     * auto computeComplete = syncManager->createSemaphore("computeComplete");
     * @endcode
     */
    virtual VkSemaphore createSemaphore(const std::string& name = "");

    /**
     * @brief Creates a fence
     * @param signaled Whether the fence should be created in signaled state
     * @param name Optional name for tracking and debugging
     * @return Created fence handle
     * @throws std::runtime_error if:
     *         - Fence creation fails
     *         - Device is invalid
     * 
     * Example:
     * @code
     * // Create a signaled fence for initial wait
     * auto fence = syncManager->createFence(true, "initialFence");
     * 
     * // Create an unsignaled fence for command buffer submission
     * auto cmdFence = syncManager->createFence(false, "cmdBufferFence");
     * @endcode
     */
    virtual VkFence createFence(bool signaled = false, const std::string& name = "");

    /**
     * @brief Waits for one or more fences
     * @param fences Vector of fence handles to wait for
     * @param waitAll Whether to wait for all fences (true) or any fence (false)
     * @param timeout Timeout in nanoseconds (UINT64_MAX for infinite wait)
     * @return VK_SUCCESS if wait succeeded, or appropriate error code
     * @throws std::runtime_error if any fence handle is invalid
     * 
     * Example:
     * @code
     * // Wait for multiple fences with timeout
     * auto result = syncManager->waitForFences(
     *     {fence1, fence2},
     *     true,  // wait for all
     *     1000000000  // 1 second timeout
     * );
     * 
     * // Wait indefinitely for a single fence
     * syncManager->waitForFences({fence}, true);
     * @endcode
     */
    virtual VkResult waitForFences(
        const std::vector<VkFence>& fences,
        bool waitAll = true,
        uint64_t timeout = UINT64_MAX);

    /**
     * @brief Resets one or more fences to unsignaled state
     * @param fences Vector of fence handles to reset
     * @throws std::runtime_error if:
     *         - Any fence handle is invalid
     *         - Reset operation fails
     * 
     * Example:
     * @code
     * // Reset fences before reuse
     * syncManager->resetFences({inFlightFence});
     * @endcode
     */
    virtual void resetFences(const std::vector<VkFence>& fences);

    /**
     * @brief Creates synchronization primitives for frame-based rendering
     * @param framesInFlight Number of frames that can be processed concurrently
     * @throws std::runtime_error if:
     *         - Creation of any synchronization object fails
     *         - framesInFlight is 0
     * 
     * Creates for each frame:
     * - Image available semaphore (GPU-GPU sync for swapchain)
     * - Render finished semaphore (GPU-GPU sync for presentation)
     * - In-flight fence (CPU-GPU sync for frame completion)
     * 
     * Example:
     * @code
     * // Setup triple buffering
     * syncManager->createFrameSynchronization(3);
     * @endcode
     */
    virtual void createFrameSynchronization(uint32_t framesInFlight);

    /**
     * @brief Get the semaphore for signaling image availability
     * @param frame Frame index
     * @return Semaphore handle for the specified frame
     * @throws std::out_of_range if frame index is invalid
     */
    VkSemaphore getImageAvailableSemaphore(uint32_t frame) const;

    /**
     * @brief Get the semaphore for signaling render completion
     * @param frame Frame index
     * @return Semaphore handle for the specified frame
     * @throws std::out_of_range if frame index is invalid
     */
    VkSemaphore getRenderFinishedSemaphore(uint32_t frame) const;

    /**
     * @brief Get the fence for frame-in-flight synchronization
     * @param frame Frame index
     * @return Fence handle for the specified frame
     * @throws std::out_of_range if frame index is invalid
     */
    VkFence getInFlightFence(uint32_t frame) const;

protected:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance

    // Per-frame synchronization objects
    std::vector<VkSemaphore> m_imageAvailableSemaphores;  ///< Semaphores for swapchain image acquisition
    std::vector<VkSemaphore> m_renderFinishedSemaphores;  ///< Semaphores for render completion
    std::vector<VkFence> m_inFlightFences;                ///< Fences for frame synchronization

    // Resource tracking maps
    std::unordered_map<std::string, VkSemaphore> m_semaphores;  ///< Named semaphores
    std::unordered_map<std::string, VkFence> m_fences;          ///< Named fences

private:
    /**
     * @brief Cleans up all synchronization objects
     * @details Called by destructor to ensure proper resource cleanup
     */
    void cleanup();

    /**
     * @brief Destroys a semaphore
     * @param semaphore Semaphore handle to destroy
     */
    void destroySemaphore(VkSemaphore semaphore);

    /**
     * @brief Destroys a fence
     * @param fence Fence handle to destroy
     */
    void destroyFence(VkFence fence);

    
};

} // namespace ev 