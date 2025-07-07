#include "EasyVulkan/Builders/BufferBuilder.hpp"

#include "EasyVulkan/Core/CommandPoolManager.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"

#include <stdexcept>

namespace ev {

BufferBuilder::BufferBuilder(VulkanDevice *device, VulkanContext *context)
    : m_device(device), m_context(context) {}

BufferBuilder &BufferBuilder::setSize(VkDeviceSize size) {
  m_size = size;
  return *this;
}

BufferBuilder &BufferBuilder::setUsage(VkBufferUsageFlags usage) {
  m_usage = usage;
  return *this;
}

BufferBuilder &
BufferBuilder::setMemoryProperties(VkMemoryPropertyFlags properties) {
  m_memoryProperties = properties;
  return *this;
}

BufferBuilder &BufferBuilder::setMemoryUsage(VmaMemoryUsage usage) {
  m_memoryUsage = usage;
  return *this;
}

BufferBuilder &BufferBuilder::setMemoryFlags(VmaAllocationCreateFlags flags) {
  m_memoryFlags = flags;
  return *this;
}

BufferBuilder &BufferBuilder::setSharingMode(VkSharingMode sharingMode) {
  m_sharingMode = sharingMode;
  return *this;
}

BufferBuilder &BufferBuilder::setQueueFamilyIndices(
    const std::vector<uint32_t> &queueFamilyIndices) {
  m_queueFamilyIndices = queueFamilyIndices;
  return *this;
}

void BufferBuilder::validateParameters() const {
  if (m_size == 0) {
    LogError("Buffer size must be greater than 0");
    throw std::runtime_error("Buffer size must be greater than 0");
  }

  if (m_usage == 0) {
    LogError("Buffer usage flags must be specified");
    throw std::runtime_error("Buffer usage flags must be specified");
  }


  if (m_sharingMode == VK_SHARING_MODE_CONCURRENT &&
      m_queueFamilyIndices.empty()) {
    LogError("Queue family indices must be specified for concurrent sharing mode");
    throw std::runtime_error(
        "Queue family indices must be specified for concurrent sharing mode");
  }

}

VkBuffer BufferBuilder::createBuffer(VmaAllocation *outAllocation) const {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = m_size;
  bufferInfo.usage = m_usage;
  bufferInfo.sharingMode = m_sharingMode;

  if (m_sharingMode == VK_SHARING_MODE_CONCURRENT) {
    bufferInfo.queueFamilyIndexCount =
        static_cast<uint32_t>(m_queueFamilyIndices.size());
    bufferInfo.pQueueFamilyIndices = m_queueFamilyIndices.data();
  }

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = m_memoryUsage;
  allocInfo.flags = m_memoryFlags;
  if (m_memoryProperties) {
    allocInfo.requiredFlags = m_memoryProperties;
  }

  VkBuffer buffer;
  VmaAllocation allocation;

  if (vmaCreateBuffer(m_device->getAllocator(), &bufferInfo, &allocInfo,
                      &buffer, &allocation, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  if (outAllocation) {
    *outAllocation = allocation;
  }

  return buffer;
}

void BufferBuilder::uploadData(VkBuffer buffer, VmaAllocation *allocation,
                               const void *data, VkDeviceSize dataSize) const {
  VmaAllocationInfo allocInfo;
  vmaGetAllocationInfo(m_device->getAllocator(), *allocation, &allocInfo);
  memcpy(allocInfo.pMappedData, data, static_cast<size_t>(dataSize));
}


VkBuffer BufferBuilder::build(const std::string &name,
                              VmaAllocation *outAllocation) {

  validateParameters();
  VkBuffer buffer = createBuffer(outAllocation);

  // Register the buffer for resource tracking if a name is provided
  if (!name.empty()) {
    m_context->getResourceManager()->registerResource(
        name, reinterpret_cast<uint64_t>(buffer),*outAllocation, m_size, m_usage, VK_OBJECT_TYPE_BUFFER);
  }

  return buffer;
}

VkBuffer BufferBuilder::buildAndInitialize(const void *data,
                                           VkDeviceSize dataSize,
                                           const std::string &name,
                                           VmaAllocation *outAllocation) {

  if (!data || dataSize == 0) {
    throw std::runtime_error("Invalid data or data size");
  }
  VmaAllocation localAllocation;
  if (!outAllocation) {
    outAllocation = &localAllocation;
  }
  
  // Set usage to VMA_MEMORY_USAGE_CPU_TO_GPU
  m_memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  // Set flags to VMA_ALLOCATION_CREATE_MAPPED_BIT
  m_memoryFlags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

  // Create the buffer
  VkBuffer buffer = build(name, outAllocation);

  // Upload the data
  uploadData(buffer, outAllocation, data, dataSize);

  return buffer;
}

} // namespace ev