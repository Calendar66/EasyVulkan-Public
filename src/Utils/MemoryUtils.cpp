#include "EasyVulkan/Utils/MemoryUtils.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include <stdexcept>
#include <cstring>

namespace ev {
namespace MemoryUtils {

uint32_t findMemoryType(
    VulkanDevice* device,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties) {
    
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void mapAndCopyData(
    VulkanDevice* device,
    VmaAllocation allocation,
    const void* data,
    VkDeviceSize size) {
    
    void* mappedData;
    if (vmaMapMemory(device->getAllocator(), allocation, &mappedData) != VK_SUCCESS) {
        throw std::runtime_error("failed to map memory!");
    }

    memcpy(mappedData, data, static_cast<size_t>(size));
    vmaUnmapMemory(device->getAllocator(), allocation);
}

void mapAndRetrieveData(
    VulkanDevice* device,
    VmaAllocation allocation,
    void* data,
    VkDeviceSize size) {
    
    void* mappedData;
    if (vmaMapMemory(device->getAllocator(), allocation, &mappedData) != VK_SUCCESS) {
        throw std::runtime_error("failed to map memory!");
    }

    memcpy(data, mappedData, static_cast<size_t>(size));
    vmaUnmapMemory(device->getAllocator(), allocation);
}

VkMemoryRequirements getBufferMemoryRequirements(
    VulkanDevice* device,
    VkBuffer buffer) {
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->getLogicalDevice(), buffer, &memRequirements);
    return memRequirements;
}

VkMemoryRequirements getImageMemoryRequirements(
    VulkanDevice* device,
    VkImage image) {
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->getLogicalDevice(), image, &memRequirements);
    return memRequirements;
}

} // namespace MemoryUtils
} // namespace ev 