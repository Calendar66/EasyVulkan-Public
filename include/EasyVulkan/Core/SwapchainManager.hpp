/**
 * @file SwapchainManager.hpp
 * @brief Swapchain management class for EasyVulkan framework
 * @details This file contains the SwapchainManager class which handles all aspects
 *          of swapchain creation and management, including image acquisition and presentation.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace ev {

class VulkanDevice;

/**
 * @class SwapchainManager
 * @brief Manages the Vulkan swapchain and related resources
 * @details SwapchainManager is responsible for:
 *          - Creating and managing the swapchain
 *          - Handling window resize events
 *          - Managing swapchain images and image views
 *          - Providing image acquisition and presentation functionality
 *
 * @note Inheritance:
 *       - Override createSwapchain() for custom surface formats or presentation modes
 *       - Override recreateSwapchain() for specialized resize handling
 *       - Override chooseSwap* methods to customize format/mode/extent selection
 *
 * Example usage:
 * @code
 * SwapchainManager swapchain(device, surface);
 * swapchain.createSwapchain();
 * 
 * // In render loop:
 * uint32_t imageIndex = swapchain.acquireNextImage(presentCompleteSemaphore);
 * // ... render to image ...
 * swapchain.presentImage(imageIndex, renderCompleteSemaphore);
 * @endcode
 */
class SwapchainManager {
public:
    /**
     * @brief Constructor for SwapchainManager
     * @param device Pointer to the VulkanDevice instance
     * @param surface Valid window surface handle
     * @throws std::runtime_error if device is nullptr or surface is VK_NULL_HANDLE
     */
    SwapchainManager(VulkanDevice* device, VkSurfaceKHR surface);
    
    /**
     * @brief Virtual destructor for proper cleanup
     * @details Destroys the swapchain and associated image views
     */
    virtual ~SwapchainManager();

    /**
     * @brief Creates the swapchain.Size will be set by chooseSwapExtent()
     * @throws std::runtime_error if swapchain creation fails
     * @details Creates a swapchain with:
     *          - Optimal surface format (typically SRGB)
     *          - Preferred presentation mode (FIFO/Mailbox)
     *          - Triple buffering when available
     */
    virtual void createSwapchain();

    /**
     * @brief Recreates the swapchain after window resize or other events
     * @param newWidth New window width
     * @param newHeight New window height
     * @throws std::runtime_error if swapchain recreation fails
     * @details
     *          1. Waits for device idle
     *          2. Cleans up old swapchain
     *          3. Creates new swapchain with updated dimensions
     *          4. Creates new image views
     */
    virtual void recreateSwapchain(uint32_t newWidth, uint32_t newHeight);

    /**
     * @brief Acquires the next available swapchain image
     * @param presentCompleteSemaphore Semaphore to signal when presentation is complete
     * @return Index of the acquired image
     * @throws std::runtime_error if image acquisition fails
     * @note Returns VK_ERROR_OUT_OF_DATE_KHR if swapchain needs recreation
     */
    uint32_t acquireNextImage(VkSemaphore presentCompleteSemaphore);

    /**
     * @brief Presents the rendered image to the display
     * @param imageIndex Index of the image to present
     * @param renderCompleteSemaphore Semaphore to wait on before presenting
     * @throws std::runtime_error if presentation fails
     * @note May return VK_ERROR_OUT_OF_DATE_KHR if swapchain needs recreation
     */
    void presentImage(uint32_t imageIndex, VkSemaphore renderCompleteSemaphore);

    /**
     * @brief Get the swapchain handle
     * @return VkSwapchainKHR Current swapchain handle
     */
    VkSwapchainKHR getSwapchain() const { return m_swapchain; }

    /**
     * @brief Get the format of swapchain images
     * @return VkFormat Format of the swapchain images
     */
    VkFormat getSwapchainImageFormat() const { return m_swapchainImageFormat; }

    /**
     * @brief Get the dimensions of swapchain images
     * @return VkExtent2D Dimensions of the swapchain images
     */
    VkExtent2D getSwapchainExtent() const { return m_swapchainExtent; }

    /**
     * @brief Get the list of swapchain images
     * @return const std::vector<VkImage>& List of swapchain image handles
     */
    const std::vector<VkImage>& getSwapchainImages() const { return m_swapchainImages; }

    /**
     * @brief Get the list of swapchain image views
     * @return const std::vector<VkImageView>& List of image view handles
     */
    const std::vector<VkImageView>& getSwapchainImageViews() const { return m_swapchainImageViews; }

    /**
     * @brief Get the number of swapchain images
     * @return uint32_t Number of swapchain images
     */
    uint32_t getSwapchainImageCount() const { return static_cast<uint32_t>(m_swapchainImages.size()); }

    /**
     * @brief Set the framebuffers for the swapchain
     * @param framebuffers Vector of framebuffer handles
     */
    void setSwapchainFramebuffers(const std::vector<VkFramebuffer>& framebuffers) { 
        m_swapchainFramebuffers = framebuffers; 
    }

    /**
     * @brief Get the framebuffers for the swapchain
     * @return const std::vector<VkFramebuffer>& List of framebuffer handles
     */
    const std::vector<VkFramebuffer>& getSwapchainFramebuffers() const { 
        return m_swapchainFramebuffers; 
    }

    /**
     * @brief Set the preferred color space for the swapchain
     * @param colorSpace The desired VkColorSpaceKHR value
     * @details This will take effect on the next swapchain creation
     */
    void setPreferredColorSpace(VkColorSpaceKHR colorSpace) { m_preferredColorSpace = colorSpace; }

    /**
     * @brief Set the image usage flags for swapchain images
     * @param imageUsage The desired VkImageUsageFlags value
     * @details This will take effect on the next swapchain creation
     *          Default is VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
     *          Common values include:
     *          - VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT: Use as color attachment
     *          - VK_IMAGE_USAGE_TRANSFER_DST_BIT: Use as transfer destination
     *          - VK_IMAGE_USAGE_SAMPLED_BIT: Use as texture
     * @note Some usage flags may not be supported by all devices
     */
    void setImageUsage(VkImageUsageFlags imageUsage) { m_imageUsage = imageUsage; }

protected:
    /**
     * @brief Chooses the optimal surface format for the swapchain
     * @param availableFormats List of available surface formats
     * @return Selected surface format
     * @details Prefers SRGB color space with B8G8R8A8 format
     */
    virtual VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /**
     * @brief Chooses the optimal presentation mode
     * @param availablePresentModes List of available presentation modes
     * @return Selected presentation mode
     * @details Prefers VK_PRESENT_MODE_MAILBOX_KHR if available,
     *          falls back to VK_PRESENT_MODE_FIFO_KHR
     */
    virtual VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);

    /**
     * @brief Chooses the optimal swap extent (dimensions)
     * @param capabilities Surface capabilities
     * @return Selected swap extent
     * @details Clamps the dimensions to the surface capabilities
     */
    virtual VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance
    VkSurfaceKHR m_surface;                  ///< Window surface handle
    VkColorSpaceKHR m_preferredColorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}; ///< Preferred color space
    VkImageUsageFlags m_imageUsage{VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT}; ///< Image usage flags for swapchain images

    VkSwapchainKHR m_swapchain;              ///< Swapchain handle
    std::vector<VkImage> m_swapchainImages;  ///< List of swapchain images
    std::vector<VkImageView> m_swapchainImageViews; ///< List of image views
    std::vector<VkFramebuffer> m_swapchainFramebuffers; ///< List of framebuffers
    VkFormat m_swapchainImageFormat;         ///< Format of swapchain images
    VkExtent2D m_swapchainExtent;           ///< Dimensions of swapchain images

    /**
     * @brief Cleans up swapchain resources
     * @details Destroys image views and swapchain
     */
    void cleanup();

    /**
     * @brief Creates image views for all swapchain images
     * @throws std::runtime_error if image view creation fails
     */
    void createImageViews();
};

} // namespace ev 