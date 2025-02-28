#include "EasyVulkan/Builders/RenderPassBuilder.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"
#include <stdexcept>

namespace ev {

RenderPassBuilder::RenderPassBuilder(VulkanDevice* device, VulkanContext* context)
    : m_device(device), m_context(context) {
}

RenderPassBuilder& RenderPassBuilder::addColorAttachment(
    VkFormat format,
    VkSampleCountFlagBits samples,
    VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp,
    VkImageLayout initialLayout,
    VkImageLayout finalLayout) {
    
    VkAttachmentDescription attachment{};
    attachment.format = format;
    attachment.samples = samples;
    attachment.loadOp = loadOp;
    attachment.storeOp = storeOp;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = initialLayout;
    attachment.finalLayout = finalLayout;

    m_attachments.push_back(attachment);
    return *this;
}

RenderPassBuilder& RenderPassBuilder::addDepthStencilAttachment(
    VkFormat format,
    VkSampleCountFlagBits samples,
    VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp,
    VkAttachmentLoadOp stencilLoadOp,
    VkAttachmentStoreOp stencilStoreOp,
    VkImageLayout initialLayout,
    VkImageLayout finalLayout) {
    
    VkAttachmentDescription attachment{};
    attachment.format = format;
    attachment.samples = samples;
    attachment.loadOp = loadOp;
    attachment.storeOp = storeOp;
    attachment.stencilLoadOp = stencilLoadOp;
    attachment.stencilStoreOp = stencilStoreOp;
    attachment.initialLayout = initialLayout;
    attachment.finalLayout = finalLayout;

    m_attachments.push_back(attachment);
    return *this;
}

RenderPassBuilder& RenderPassBuilder::beginSubpass(VkPipelineBindPoint bindPoint) {
    if (m_inSubpass) {
        throw std::runtime_error("Cannot begin a new subpass before ending the current one");
    }

    m_inSubpass = true;
    m_hasDepthStencil = false;
    m_colorRefs.clear();
    m_inputRefs.clear();
    m_preserveRefs.clear();

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = bindPoint;
    m_subpasses.push_back(subpass);

    return *this;
}

RenderPassBuilder& RenderPassBuilder::addColorReference(
    uint32_t attachment,
    VkImageLayout layout) {
    
    if (!m_inSubpass) {
        throw std::runtime_error("Must begin a subpass before adding color references");
    }

    if (attachment >= m_attachments.size()) {
        throw std::runtime_error("Color attachment reference out of range");
    }

    VkAttachmentReference reference{};
    reference.attachment = attachment;
    reference.layout = layout;
    m_colorRefs.push_back(reference);

    return *this;
}

RenderPassBuilder& RenderPassBuilder::setDepthStencilReference(
    uint32_t attachment,
    VkImageLayout layout) {
    
    if (!m_inSubpass) {
        throw std::runtime_error("Must begin a subpass before setting depth/stencil reference");
    }

    if (attachment >= m_attachments.size()) {
        throw std::runtime_error("Depth/stencil attachment reference out of range");
    }

    if (m_hasDepthStencil) {
        throw std::runtime_error("Subpass already has a depth/stencil attachment");
    }

    m_depthStencilRef.attachment = attachment;
    m_depthStencilRef.layout = layout;
    m_hasDepthStencil = true;

    return *this;
}

RenderPassBuilder& RenderPassBuilder::addInputReference(
    uint32_t attachment,
    VkImageLayout layout) {
    
    if (!m_inSubpass) {
        throw std::runtime_error("Must begin a subpass before adding input references");
    }

    if (attachment >= m_attachments.size()) {
        throw std::runtime_error("Input attachment reference out of range");
    }

    VkAttachmentReference reference{};
    reference.attachment = attachment;
    reference.layout = layout;
    m_inputRefs.push_back(reference);

    return *this;
}

RenderPassBuilder& RenderPassBuilder::addPreserveReference(uint32_t attachment) {
    if (!m_inSubpass) {
        throw std::runtime_error("Must begin a subpass before adding preserve references");
    }

    if (attachment >= m_attachments.size()) {
        throw std::runtime_error("Preserve attachment reference out of range");
    }

    m_preserveRefs.push_back(attachment);
    return *this;
}

RenderPassBuilder& RenderPassBuilder::endSubpass() {
    if (!m_inSubpass) {
        throw std::runtime_error("No subpass to end");
    }

    finalizeCurrentSubpass();
    m_inSubpass = false;
    return *this;
}

RenderPassBuilder& RenderPassBuilder::addDependency(
    uint32_t srcSubpass,
    uint32_t dstSubpass,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask,
    VkDependencyFlags dependencyFlags) {
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = srcSubpass;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = srcStageMask;
    dependency.dstStageMask = dstStageMask;
    dependency.srcAccessMask = srcAccessMask;
    dependency.dstAccessMask = dstAccessMask;
    dependency.dependencyFlags = dependencyFlags;

    m_dependencies.push_back(dependency);
    return *this;
}

void RenderPassBuilder::validateState() const {
    if (m_attachments.empty()) {
        throw std::runtime_error("No attachments specified for render pass");
    }

    if (m_subpasses.empty()) {
        throw std::runtime_error("No subpasses specified for render pass");
    }

    if (m_inSubpass) {
        throw std::runtime_error("Current subpass not ended");
    }
}

void RenderPassBuilder::finalizeCurrentSubpass() {
    // Store current subpass data
    m_allColorRefs.push_back(m_colorRefs);
    m_allInputRefs.push_back(m_inputRefs);
    m_allPreserveRefs.push_back(m_preserveRefs);

    // Update the last subpass description
    VkSubpassDescription& subpass = m_subpasses.back();
    
    if (!m_colorRefs.empty()) {
        subpass.colorAttachmentCount = static_cast<uint32_t>(m_colorRefs.size());
        subpass.pColorAttachments = m_allColorRefs.back().data();
    }

    if (m_hasDepthStencil) {
        subpass.pDepthStencilAttachment = &m_depthStencilRef;
    }

    if (!m_inputRefs.empty()) {
        subpass.inputAttachmentCount = static_cast<uint32_t>(m_inputRefs.size());
        subpass.pInputAttachments = m_allInputRefs.back().data();
    }

    if (!m_preserveRefs.empty()) {
        subpass.preserveAttachmentCount = static_cast<uint32_t>(m_preserveRefs.size());
        subpass.pPreserveAttachments = m_allPreserveRefs.back().data();
    }
}

VkRenderPass RenderPassBuilder::build(const std::string& name) {
    validateState();

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(m_attachments.size());
    renderPassInfo.pAttachments = m_attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(m_subpasses.size());
    renderPassInfo.pSubpasses = m_subpasses.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(m_dependencies.size());
    renderPassInfo.pDependencies = m_dependencies.data();

    VkRenderPass renderPass;
    if (vkCreateRenderPass(m_device->getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    // Register the render pass for resource tracking if a name is provided
    if (!name.empty()) {
        auto *resourceManager = m_context->getResourceManager();
        resourceManager->registerResource(name,
                                        reinterpret_cast<uint64_t>(renderPass),
                                        VK_OBJECT_TYPE_RENDER_PASS);
    }

    return renderPass;
}

} // namespace ev 