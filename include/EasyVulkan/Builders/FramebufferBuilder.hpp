/**
 * @file FramebufferBuilder.hpp
 * @brief Builder class for creating framebuffers in EasyVulkan framework
 * @details This file contains the FramebufferBuilder class which provides a fluent interface
 *          for creating Vulkan framebuffers with attachments and dimensions.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace ev {

class VulkanDevice;
class VulkanContext;

/**
 * @class FramebufferBuilder
 * @brief Fluent builder interface for creating Vulkan framebuffers
 * @details FramebufferBuilder simplifies framebuffer creation by:
 *          - Providing a fluent interface for configuration
 *          - Managing attachment image views
 *          - Handling framebuffer dimensions
 *          - Providing sensible defaults for common use cases
 *          - Validating framebuffer configuration
 *
 * Common usage patterns:
 * @code
 * // Create a simple framebuffer with color and depth
 * auto framebuffer = framebufferBuilder
 *     ->addAttachment(colorImageView)
 *     ->addAttachment(depthImageView)
 *     ->setDimensions(width, height)
 *     ->build(renderPass, "mainFramebuffer");
 *
 * // Create a framebuffer for multisampling
 * auto msaaFramebuffer = framebufferBuilder
 *     ->addAttachment(msaaColorImageView)
 *     ->addAttachment(msaaDepthImageView)
 *     ->addAttachment(resolveImageView)
 *     ->setDimensions(width, height)
 *     ->build(renderPass, "msaaFramebuffer");
 *
 * // Create a layered framebuffer (e.g., for cubemap rendering)
 * auto cubemapFramebuffer = framebufferBuilder
 *     ->addAttachment(cubemapImageView)
 *     ->setDimensions(size, size, 6)  // 6 layers for cubemap
 *     ->build(renderPass, "cubemapFramebuffer");
 * @endcode
 */
class FramebufferBuilder {
public:
    /**
     * @brief Constructor for FramebufferBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    explicit FramebufferBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Virtual destructor for proper cleanup
     */
    ~FramebufferBuilder() = default;

    /**
     * @brief Adds an attachment to the framebuffer
     * @param attachment Image view handle for the attachment
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Image view handle is invalid
     *         - Too many attachments are added
     *
     * @note Attachments must be added in the same order as they are declared
     *       in the render pass this framebuffer will be used with.
     */
    FramebufferBuilder& addAttachment(VkImageView attachment);

    /**
     * @brief Sets the dimensions of the framebuffer
     * @param width Width in pixels
     * @param height Height in pixels
     * @param layers Number of array layers (default: 1)
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Width or height is 0
     *         - Layers is 0
     *         - Dimensions exceed device limits
     *
     * @note For non-layered rendering, use layers = 1
     *       For cubemap rendering, use layers = 6
     *       For array textures, set layers to array size
     */
    FramebufferBuilder& setDimensions(
        uint32_t width,
        uint32_t height,
        uint32_t layers = 1);

    /**
     * @brief Builds the framebuffer with current configuration
     * @param renderPass Render pass the framebuffer will be used with
     * @param name Optional name for resource tracking
     * @return Created framebuffer handle
     * @throws std::runtime_error if:
     *         - No attachments have been added
     *         - Dimensions are not set
     *         - Render pass is invalid
     *         - Number of attachments doesn't match render pass
     *         - Framebuffer creation fails
     *
     * @note The framebuffer must be compatible with the specified render pass:
     *       - Same number of attachments
     *       - Compatible attachment formats
     *       - Compatible sample counts
     */
    VkFramebuffer build(
        VkRenderPass renderPass,
        const std::string& name = "");

private:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance
    VulkanContext* m_context;                ///< Pointer to VulkanContext instance

    // Framebuffer parameters
    std::vector<VkImageView> m_attachments;  ///< Attachment image views
    uint32_t m_width{0};                     ///< Framebuffer width
    uint32_t m_height{0};                    ///< Framebuffer height
    uint32_t m_layers{1};                    ///< Number of array layers

    /**
     * @brief Validates the current builder state
     * @throws std::runtime_error if:
     *         - No attachments have been added
     *         - Dimensions are not set
     *         - Parameters exceed device limits
     */
    void validateParameters() const;
};

} // namespace ev 