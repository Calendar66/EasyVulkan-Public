#include "EasyVulkan/Builders/CommandBufferBuilder.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"

#include <stdexcept>

namespace ev {

CommandBufferBuilder::CommandBufferBuilder(VulkanDevice* device, VulkanContext* context)
    : m_device(device)
    , m_context(context) {
}

CommandBufferBuilder& CommandBufferBuilder::setLevel(VkCommandBufferLevel level) {
    m_level = level;
    return *this;
}

CommandBufferBuilder& CommandBufferBuilder::setCommandPool(VkCommandPool pool) {
    m_commandPool = pool;
    return *this;
}

CommandBufferBuilder& CommandBufferBuilder::setUsageFlags(VkCommandBufferUsageFlags flags) {
    m_usageFlags = flags;
    return *this;
}

CommandBufferBuilder& CommandBufferBuilder::setCount(uint32_t count) {
    m_count = count;
    return *this;
}

void CommandBufferBuilder::validateParameters() const {
    if (!m_device) {
        LogError("Device must be specified");
        throw std::runtime_error("Device must be specified");
    }


    if (m_commandPool == VK_NULL_HANDLE) {
        LogError("Command pool must be specified");
        throw std::runtime_error("Command pool must be specified");
    }


    if (m_count == 0) {
        LogError("Command buffer count must be greater than 0");
        throw std::runtime_error("Command buffer count must be greater than 0");
    }
}

std::vector<VkCommandBuffer> CommandBufferBuilder::createCommandBuffers(const std::vector<std::string>& names) const {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = m_level;
    allocInfo.commandBufferCount = m_count;

    std::vector<VkCommandBuffer> commandBuffers(m_count);
    if (vkAllocateCommandBuffers(m_device->getLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    // Register command buffers with resource manager if names are provided
    if (!names.empty()) {
        for (size_t i = 0; i < std::min(names.size(), commandBuffers.size()); ++i) {
            if (!names[i].empty()) {
                auto* resourceManager = m_context->getResourceManager();
                resourceManager->registerResource2(
                    names[i],
                    reinterpret_cast<uint64_t>(commandBuffers[i]),
                    reinterpret_cast<uint64_t>(m_commandPool),
                    VK_OBJECT_TYPE_COMMAND_BUFFER);
            }
        }
    }

    return commandBuffers;
}

VkCommandBuffer CommandBufferBuilder::build(const std::string& name) {
    validateParameters();
    m_count = 1;
    std::vector<std::string> names;
    if (!name.empty()) {
        names.push_back(name);
    }
    return createCommandBuffers(names)[0];
}

std::vector<VkCommandBuffer> CommandBufferBuilder::buildMultiple(const std::vector<std::string>& names) {
    validateParameters();
    return createCommandBuffers(names);
}

} // namespace ev
