#include "EasyVulkan/Builders/FramebufferBuilder.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"
#include <stdexcept>

namespace ev {

FramebufferBuilder::FramebufferBuilder(VulkanDevice* device, VulkanContext* context)
    : m_device(device), m_context(context) {
}

FramebufferBuilder& FramebufferBuilder::addAttachment(VkImageView attachment) {
    m_attachments.push_back(attachment);
    return *this;
}

FramebufferBuilder& FramebufferBuilder::setDimensions(
    uint32_t width,
    uint32_t height,
    uint32_t layers) {
    
    m_width = width;
    m_height = height;
    m_layers = layers;
    return *this;
}

void FramebufferBuilder::validateParameters() const {
    if (m_attachments.empty()) {
        throw std::runtime_error("No attachments specified for framebuffer");
    }

    if (m_width == 0 || m_height == 0) {
        throw std::runtime_error("Invalid framebuffer dimensions");
    }

    if (m_layers == 0) {
        throw std::runtime_error("Invalid number of framebuffer layers");
    }
}

VkFramebuffer FramebufferBuilder::build(
    VkRenderPass renderPass,
    const std::string& name) {
    
    validateParameters();

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(m_attachments.size());
    framebufferInfo.pAttachments = m_attachments.data();
    framebufferInfo.width = m_width;
    framebufferInfo.height = m_height;
    framebufferInfo.layers = m_layers;

    VkFramebuffer framebuffer;
    if (vkCreateFramebuffer(m_device->getLogicalDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }

    // Register the framebuffer for resource tracking if a name is provided
    if (!name.empty()) {
        auto *resourceManager = m_context->getResourceManager();
        resourceManager->registerResource(name,
                                        reinterpret_cast<uint64_t>(framebuffer),
                                        VK_OBJECT_TYPE_FRAMEBUFFER);
    }

    return framebuffer;
}

} // namespace ev 