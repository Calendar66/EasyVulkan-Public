#include "EasyVulkan/Core/CommandPoolManager.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace ev {

CommandPoolManager::CommandPoolManager(VulkanDevice* device)
    : m_device(device)
    , m_singleTimeCommandPool(VK_NULL_HANDLE) {
    createSingleTimeCommandPool();
}

CommandPoolManager::~CommandPoolManager() {
    cleanup();
}

VkCommandPool CommandPoolManager::createCommandPool(
    uint32_t queueFamilyIndex,
    VkCommandPoolCreateFlags flags) {
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = flags;

    VkCommandPool commandPool;
    if (vkCreateCommandPool(m_device->getLogicalDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

    m_commandPools[queueFamilyIndex].push_back(commandPool);
    return commandPool;
}

std::vector<VkCommandBuffer> CommandPoolManager::allocateCommandBuffers(
    VkCommandPool pool,
    VkCommandBufferLevel level,
    uint32_t count) {
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = count;

    std::vector<VkCommandBuffer> commandBuffers(count);
    if (vkAllocateCommandBuffers(m_device->getLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    return commandBuffers;
}

VkCommandBuffer CommandPoolManager::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_singleTimeCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device->getLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void CommandPoolManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkQueue queue = m_device->getGraphicsQueue();
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(m_device->getLogicalDevice(), m_singleTimeCommandPool, 1, &commandBuffer);
}

void CommandPoolManager::freeCommandBuffers(
    VkCommandPool pool,
    const std::vector<VkCommandBuffer>& commandBuffers) {
    
    vkFreeCommandBuffers(
        m_device->getLogicalDevice(),
        pool,
        static_cast<uint32_t>(commandBuffers.size()),
        commandBuffers.data());
}

void CommandPoolManager::resetCommandPool(
    VkCommandPool pool,
    VkCommandPoolResetFlags flags) {
    
    vkResetCommandPool(m_device->getLogicalDevice(), pool, flags);
}

void CommandPoolManager::clearCommandBuffers(
    VkCommandPool pool,
    ResourceManager* resourceManager) {
    
    if (pool == VK_NULL_HANDLE) {
        throw std::runtime_error("Invalid command pool handle");
    }
    
    if (!resourceManager) {
        throw std::runtime_error("ResourceManager cannot be null");
    }
    
    VkDevice device = m_device->getLogicalDevice();
    
    // Collect command buffers to free
    std::vector<VkCommandBuffer> commandBuffersToFree;
    std::vector<std::string> keysToRemove;
    
    // Find all command buffers associated with this pool
    for (const auto& pair : resourceManager->m_commandBuffers) {
        const auto& info = pair.second;
        if (info.commandPool == pool) {
            commandBuffersToFree.push_back(info.commandBuffer);
            keysToRemove.push_back(pair.first);
        }
    }
    
    // Free the command buffers if any were found
    if (!commandBuffersToFree.empty()) {
        vkFreeCommandBuffers(
            device,
            pool,
            static_cast<uint32_t>(commandBuffersToFree.size()),
            commandBuffersToFree.data()
        );
        
        // Remove the freed command buffers from ResourceManager tracking
        for (const auto& key : keysToRemove) {
            resourceManager->m_commandBuffers.erase(key);
        }
    }
}

void CommandPoolManager::cleanup() {
    VkDevice device = m_device->getLogicalDevice();

    // Cleanup all command pools
    for (const auto& pair : m_commandPools) {
        for (VkCommandPool pool : pair.second) {
            vkDestroyCommandPool(device, pool, nullptr);
        }
    }
    m_commandPools.clear();

    // Cleanup single-time command pool
    if (m_singleTimeCommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, m_singleTimeCommandPool, nullptr);
        m_singleTimeCommandPool = VK_NULL_HANDLE;
    }
}

void CommandPoolManager::createSingleTimeCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = 0; // Assuming graphics queue family index is 0

    if (vkCreateCommandPool(m_device->getLogicalDevice(), &poolInfo, nullptr, &m_singleTimeCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool for single time commands!");
    }
}

} // namespace ev 