#include "EasyVulkan/Core/SynchronizationManager.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include <stdexcept>

namespace ev {

SynchronizationManager::SynchronizationManager(VulkanDevice* device)
    : m_device(device) {
}

SynchronizationManager::~SynchronizationManager() {
    cleanup();
}

VkSemaphore SynchronizationManager::createSemaphore(const std::string& name) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore;
    if (vkCreateSemaphore(m_device->getLogicalDevice(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }

    if (!name.empty()) {
        m_semaphores[name] = semaphore;
    }

    return semaphore;
}

VkFence SynchronizationManager::createFence(bool signaled, const std::string& name) {
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signaled) {
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VkFence fence;
    if (vkCreateFence(m_device->getLogicalDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    }

    if (!name.empty()) {
        m_fences[name] = fence;
    }

    return fence;
}

VkResult SynchronizationManager::waitForFences(
    const std::vector<VkFence>& fences,
    bool waitAll,
    uint64_t timeout) {
    
    return vkWaitForFences(
        m_device->getLogicalDevice(),
        static_cast<uint32_t>(fences.size()),
        fences.data(),
        waitAll,
        timeout);
}

void SynchronizationManager::resetFences(const std::vector<VkFence>& fences) {
    vkResetFences(
        m_device->getLogicalDevice(),
        static_cast<uint32_t>(fences.size()),
        fences.data());
}

void SynchronizationManager::createFrameSynchronization(uint32_t framesInFlight) {
    m_imageAvailableSemaphores.resize(framesInFlight);
    m_renderFinishedSemaphores.resize(framesInFlight);
    m_inFlightFences.resize(framesInFlight);

    for (size_t i = 0; i < framesInFlight; i++) {
        m_imageAvailableSemaphores[i] = createSemaphore();
        m_renderFinishedSemaphores[i] = createSemaphore();
        m_inFlightFences[i] = createFence(true);
    }
}

VkSemaphore SynchronizationManager::getImageAvailableSemaphore(uint32_t frame) const {
    if (frame >= m_imageAvailableSemaphores.size()) {
        throw std::runtime_error("frame index out of range!");
    }
    return m_imageAvailableSemaphores[frame];
}

VkSemaphore SynchronizationManager::getRenderFinishedSemaphore(uint32_t frame) const {
    if (frame >= m_renderFinishedSemaphores.size()) {
        throw std::runtime_error("frame index out of range!");
    }
    return m_renderFinishedSemaphores[frame];
}

VkFence SynchronizationManager::getInFlightFence(uint32_t frame) const {
    if (frame >= m_inFlightFences.size()) {
        throw std::runtime_error("frame index out of range!");
    }
    return m_inFlightFences[frame];
}

void SynchronizationManager::cleanup() {
    VkDevice device = m_device->getLogicalDevice();

    // Cleanup per-frame synchronization objects
    for (auto semaphore : m_imageAvailableSemaphores) {
        destroySemaphore(semaphore);
    }
    m_imageAvailableSemaphores.clear();

    for (auto semaphore : m_renderFinishedSemaphores) {
        destroySemaphore(semaphore);
    }
    m_renderFinishedSemaphores.clear();

    for (auto fence : m_inFlightFences) {
        destroyFence(fence);
    }
    m_inFlightFences.clear();

    // Cleanup named synchronization objects
    for (const auto& pair : m_semaphores) {
        destroySemaphore(pair.second);
    }
    m_semaphores.clear();

    for (const auto& pair : m_fences) {
        destroyFence(pair.second);
    }
    m_fences.clear();
}

void SynchronizationManager::destroySemaphore(VkSemaphore semaphore) {
    if (semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device->getLogicalDevice(), semaphore, nullptr);
    }
}

void SynchronizationManager::destroyFence(VkFence fence) {
    if (fence != VK_NULL_HANDLE) {
        vkDestroyFence(m_device->getLogicalDevice(), fence, nullptr);
    }
}

} // namespace ev 