/**
 * @file RenderPassBuilder.hpp
 * @brief Builder class for creating render passes in EasyVulkan framework
 * @details This file contains the RenderPassBuilder class which provides a fluent interface
 *          for creating Vulkan render passes with attachments, subpasses, and dependencies.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace ev {

class VulkanDevice;
class VulkanContext;

/**
 * @class RenderPassBuilder
 * @brief Fluent builder interface for creating Vulkan render passes
 * @details RenderPassBuilder simplifies render pass creation by:
 *          - Providing a fluent interface for configuration
 *          - Managing attachments, subpasses, and dependencies
 *          - Providing sensible defaults for common use cases
 *          - Validating render pass configuration
 *          - Supporting multiple subpasses and complex dependencies
 *
 * Common usage patterns:
 * @code
 * // Create a simple render pass with color and depth
 * auto renderPass = renderPassBuilder
 *     .addColorAttachment(swapchainFormat)
 *     .addDepthStencilAttachment(depthFormat)
 *     .beginSubpass()
 *     .addColorReference(0)
 *     .setDepthStencilReference(1)
 *     .endSubpass()
 *     .build("mainRenderPass");
 *
 * // Create a render pass with multiple subpasses
 * auto renderPass = renderPassBuilder
 *     .addColorAttachment(colorFormat)
 *     .addDepthStencilAttachment(depthFormat)
 *     // First subpass
 *     .beginSubpass()
 *     .addColorReference(0)
 *     .setDepthStencilReference(1)
 *     .endSubpass()
 *     // Second subpass using first pass output
 *     .beginSubpass()
 *     .addInputReference(0)
 *     .addColorReference(2)
 *     .endSubpass()
 *     // Add dependency between subpasses
 *     .addDependency(0, 1,
 *         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
 *         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
 *         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
 *         VK_ACCESS_SHADER_READ_BIT)
 *     .build("multiPassRender");
 * @endcode
 */
class RenderPassBuilder {
public:
    /**
     * @brief Constructor for RenderPassBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    explicit RenderPassBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Virtual destructor for proper cleanup
     */
    ~RenderPassBuilder() = default;

    /**
     * @brief Adds a color attachment to the render pass
     * @param format Format of the color attachment
     * @param samples Number of samples for multisampling
     * @param loadOp How to handle attachment contents at start
     * @param storeOp How to handle attachment contents at end
     * @param initialLayout Initial layout of attachment
     * @param finalLayout Final layout of attachment
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if attachment parameters are invalid
     */
    RenderPassBuilder& addColorAttachment(
        VkFormat format,
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    /**
     * @brief Adds a depth/stencil attachment to the render pass
     * @param format Format of the depth/stencil attachment
     * @param samples Number of samples for multisampling
     * @param loadOp How to handle depth contents at start
     * @param storeOp How to handle depth contents at end
     * @param stencilLoadOp How to handle stencil contents at start
     * @param stencilStoreOp How to handle stencil contents at end
     * @param initialLayout Initial layout of attachment
     * @param finalLayout Final layout of attachment
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Attachment parameters are invalid
     *         - Multiple depth/stencil attachments are added
     */
    RenderPassBuilder& addDepthStencilAttachment(
        VkFormat format,
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    /**
     * @brief Begins a new subpass in the render pass
     * @param bindPoint Pipeline bind point (graphics or compute)
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if already in a subpass
     */
    RenderPassBuilder& beginSubpass(
        VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    /**
     * @brief Adds a color attachment reference to the current subpass
     * @param attachment Index of the attachment to reference
     * @param layout Layout the attachment will be in during the subpass
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Not in a subpass
     *         - Attachment index is invalid
     *         - Attachment is not a color attachment
     */
    RenderPassBuilder& addColorReference(
        uint32_t attachment,
        VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    /**
     * @brief Sets the depth/stencil attachment reference for the current subpass
     * @param attachment Index of the attachment to reference
     * @param layout Layout the attachment will be in during the subpass
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Not in a subpass
     *         - Attachment index is invalid
     *         - Attachment is not a depth/stencil attachment
     *         - Multiple depth/stencil references are set
     */
    RenderPassBuilder& setDepthStencilReference(
        uint32_t attachment,
        VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    /**
     * @brief Adds an input attachment reference to the current subpass
     * @param attachment Index of the attachment to reference
     * @param layout Layout the attachment will be in during the subpass
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Not in a subpass
     *         - Attachment index is invalid
     */
    RenderPassBuilder& addInputReference(
        uint32_t attachment,
        VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    /**
     * @brief Adds a preserve attachment reference to the current subpass
     * @param attachment Index of the attachment to preserve
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Not in a subpass
     *         - Attachment index is invalid
     */
    RenderPassBuilder& addPreserveReference(uint32_t attachment);

    /**
     * @brief Ends the current subpass
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if not in a subpass
     */
    RenderPassBuilder& endSubpass();

    /**
     * @brief Adds a dependency between subpasses
     * @param srcSubpass Source subpass index (VK_SUBPASS_EXTERNAL for external)
     * @param dstSubpass Destination subpass index (VK_SUBPASS_EXTERNAL for external)
     * @param srcStageMask Pipeline stages to wait on in source subpass
     * @param dstStageMask Pipeline stages to wait in destination subpass
     * @param srcAccessMask Memory access to wait on in source subpass
     * @param dstAccessMask Memory access to wait for in destination subpass
     * @param dependencyFlags Additional dependency flags
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Subpass indices are invalid
     *         - Stage or access masks are invalid
     */
    RenderPassBuilder& addDependency(
        uint32_t srcSubpass,
        uint32_t dstSubpass,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkDependencyFlags dependencyFlags = 0);

    /**
     * @brief Builds the render pass with current configuration
     * @param name Optional name for resource tracking
     * @return Created render pass handle
     * @throws std::runtime_error if:
     *         - No attachments have been added
     *         - No subpasses have been added
     *         - Current subpass is not ended
     *         - Render pass creation fails
     */
    VkRenderPass build(const std::string& name = "");

private:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance
    VulkanContext* m_context;                ///< Pointer to VulkanContext instance

    // Render pass components
    std::vector<VkAttachmentDescription> m_attachments;    ///< Attachment descriptions
    std::vector<VkSubpassDescription> m_subpasses;         ///< Subpass descriptions
    std::vector<VkSubpassDependency> m_dependencies;       ///< Subpass dependencies

    // Current subpass state
    std::vector<VkAttachmentReference> m_colorRefs;        ///< Color attachment references
    VkAttachmentReference m_depthStencilRef;              ///< Depth/stencil attachment reference
    std::vector<VkAttachmentReference> m_inputRefs;        ///< Input attachment references
    std::vector<uint32_t> m_preserveRefs;                 ///< Preserve attachment references
    bool m_hasDepthStencil{false};                        ///< Whether depth/stencil is set
    bool m_inSubpass{false};                              ///< Whether in a subpass

    // Storage for subpass data
    std::vector<std::vector<VkAttachmentReference>> m_allColorRefs;    ///< All color references
    std::vector<std::vector<VkAttachmentReference>> m_allInputRefs;    ///< All input references
    std::vector<std::vector<uint32_t>> m_allPreserveRefs;             ///< All preserve references

    /**
     * @brief Validates the current builder state
     * @throws std::runtime_error if state is invalid
     */
    void validateState() const;

    /**
     * @brief Finalizes the current subpass configuration
     * @throws std::runtime_error if subpass configuration is invalid
     */
    void finalizeCurrentSubpass();
};

} // namespace ev 