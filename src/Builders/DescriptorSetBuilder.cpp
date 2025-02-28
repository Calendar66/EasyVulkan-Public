#include "EasyVulkan/Builders/DescriptorSetBuilder.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include <stdexcept>
#include <unordered_map>

namespace ev {

DescriptorSetBuilder::DescriptorSetBuilder(VulkanDevice *device,
                                           VulkanContext *context)
    : m_device(device), m_context(context) {}

DescriptorSetBuilder &
DescriptorSetBuilder::addBinding(uint32_t binding, VkDescriptorType type,
                                 uint32_t count,
                                 VkShaderStageFlags stageFlags) {

  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = type;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  layoutBinding.pImmutableSamplers = nullptr;

  m_layoutBindings.push_back(layoutBinding);
  return *this;
}

DescriptorSetBuilder &DescriptorSetBuilder::addBufferDescriptor(
    uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range,
    VkDescriptorType type) {

  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = buffer;
  bufferInfo.offset = offset;
  bufferInfo.range = range;
  m_bufferInfos.push_back(bufferInfo);

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstBinding = binding;
  write.dstArrayElement = 0;
  write.descriptorType = type;
  write.descriptorCount = 1;
  write.pBufferInfo = &m_bufferInfos.back();

  m_writes.push_back(write);
  return *this;
}

DescriptorSetBuilder &DescriptorSetBuilder::addImageDescriptor(
    uint32_t binding, VkImageView imageView, VkSampler sampler,
    VkImageLayout imageLayout, VkDescriptorType type) {

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = imageLayout;
  imageInfo.imageView = imageView;
  if (sampler != VK_NULL_HANDLE) {  
    imageInfo.sampler = sampler;
  }
  m_imageInfos.push_back(imageInfo);


  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstBinding = binding;
  write.dstArrayElement = 0;
  write.descriptorType = type;
  write.descriptorCount = 1;
  write.pImageInfo = &m_imageInfos.back();


  m_writes.push_back(write);
  return *this;
}

DescriptorSetBuilder &DescriptorSetBuilder::addStorageImageDescriptor(
    uint32_t binding, VkImageView imageView, VkImageLayout imageLayout) {

  return addImageDescriptor(binding, imageView, VK_NULL_HANDLE, imageLayout,
                            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
}


void DescriptorSetBuilder::validateBindings() const {
  if (m_layoutBindings.empty()) {
    throw std::runtime_error("No descriptor set bindings specified");
  }

  // Check for duplicate bindings
  std::unordered_map<uint32_t, VkDescriptorType> bindingTypes;
  for (const auto &binding : m_layoutBindings) {
    auto [it, inserted] =
        bindingTypes.insert({binding.binding, binding.descriptorType});
    if (!inserted) {
      throw std::runtime_error(
          "Duplicate binding number in descriptor set layout");
    }
  }

  // Validate write descriptors against bindings
  for (const auto &write : m_writes) {
    auto it = bindingTypes.find(write.dstBinding);
    if (it == bindingTypes.end()) {
      throw std::runtime_error(
          "Write descriptor binding does not exist in layout");
    }
    if (it->second != write.descriptorType) {
      LogError("Write descriptor type does not match layout binding type");
      throw std::runtime_error(
          "Write descriptor type does not match layout binding type");
    }
  }
}

VkDescriptorPool DescriptorSetBuilder::createPool() const {
  std::unordered_map<VkDescriptorType, uint32_t> typeCount;
  for (const auto &binding : m_layoutBindings) {
    typeCount[binding.descriptorType] += binding.descriptorCount;
  }

  std::vector<VkDescriptorPoolSize> poolSizes;
  for (const auto &[type, count] : typeCount) {
    poolSizes.push_back({type, count});
  }

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = 1; // Only allocating one set from this pool

  VkDescriptorPool pool;
  if (vkCreateDescriptorPool(m_device->getLogicalDevice(), &poolInfo, nullptr,
                             &pool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }

  return pool;
}

void DescriptorSetBuilder::updateDescriptorSet(
    VkDescriptorSet descriptorSet) const {
  // Update all write descriptor sets with the created descriptor set
  std::vector<VkWriteDescriptorSet> writes = m_writes;
  for (auto &write : writes) {
    write.dstSet = descriptorSet;
  }

  vkUpdateDescriptorSets(m_device->getLogicalDevice(),
                         static_cast<uint32_t>(writes.size()), writes.data(), 0,
                         nullptr);
}

VkDescriptorSetLayout
DescriptorSetBuilder::createLayout(const std::string &name) {
  validateBindings();

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(m_layoutBindings.size());
  layoutInfo.pBindings = m_layoutBindings.data();

  VkDescriptorSetLayout layout;
  if (vkCreateDescriptorSetLayout(m_device->getLogicalDevice(), &layoutInfo,
                                  nullptr, &layout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  // Register the layout for resource tracking if a name is provided
  if (!name.empty()) {
    auto *resourceManager = m_context->getResourceManager();
    resourceManager->registerResource(name, reinterpret_cast<uint64_t>(layout),
                                      VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);
  }

  return layout;
}

VkDescriptorSet DescriptorSetBuilder::build(VkDescriptorSetLayout layout,
                                            const std::string &name) {

  validateBindings();

  // Create a descriptor pool
  VkDescriptorPool pool = createPool();

  // Allocate descriptor set
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = pool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &layout;

  VkDescriptorSet descriptorSet;
  if (vkAllocateDescriptorSets(m_device->getLogicalDevice(), &allocInfo,
                               &descriptorSet) != VK_SUCCESS) {
    vkDestroyDescriptorPool(m_device->getLogicalDevice(), pool, nullptr);
    throw std::runtime_error("failed to allocate descriptor set!");
  }

  // Update the descriptor set
  updateDescriptorSet(descriptorSet);

  // Register the descriptor set for resource tracking if a name is provided
  if (!name.empty()) {
    auto *resourceManager = m_context->getResourceManager();
    resourceManager->registerResource(name,
                                      reinterpret_cast<uint64_t>(descriptorSet),
                                      VK_OBJECT_TYPE_DESCRIPTOR_SET);
  }

  return descriptorSet;
}

VkDescriptorSet DescriptorSetBuilder::buildWithLayout(const std::string &name) {
  VkDescriptorSetLayout layout = createLayout(name + "_layout");
  return build(layout, name);
}

} // namespace ev