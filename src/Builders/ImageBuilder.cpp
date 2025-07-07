#include "EasyVulkan/Builders/ImageBuilder.hpp"

#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"

#include "EasyVulkan/Core/ResourceManager.hpp"
#include "EasyVulkan/Core/CommandPoolManager.hpp"
#include "EasyVulkan/Builders/BufferBuilder.hpp"
#include "EasyVulkan/Utils/ResourceUtils.hpp"
#include <stdexcept>


namespace ev {

ImageBuilder::ImageBuilder(VulkanDevice* device,VulkanContext* context)
    : m_device(device),m_context(context) {
}

ImageBuilder& ImageBuilder::setImageType(VkImageType imageType) {
    m_imageType = imageType;
    return *this;
}

ImageBuilder& ImageBuilder::setFormat(VkFormat format) {
    m_format = format;
    return *this;
}

ImageBuilder& ImageBuilder::setExtent(uint32_t width, uint32_t height, uint32_t depth) {
    m_extent.width = width;
    m_extent.height = height;
    m_extent.depth = depth;
    return *this;
}

ImageBuilder& ImageBuilder::setMipLevels(uint32_t mipLevels) {
    m_mipLevels = mipLevels;
    return *this;
}

ImageBuilder& ImageBuilder::setArrayLayers(uint32_t arrayLayers) {
    m_arrayLayers = arrayLayers;
    return *this;
}

ImageBuilder& ImageBuilder::setSamples(VkSampleCountFlagBits samples) {
    m_samples = samples;
    return *this;
}

ImageBuilder& ImageBuilder::setTiling(VkImageTiling tiling) {
    m_tiling = tiling;
    return *this;
}

ImageBuilder& ImageBuilder::setUsage(VkImageUsageFlags usage) {
    m_usage = usage;
    return *this;
}

ImageBuilder& ImageBuilder::setMemoryProperties(VkMemoryPropertyFlags properties) {
    m_memoryProperties = properties;
    return *this;
}

ImageBuilder& ImageBuilder::setMemoryUsage(VmaMemoryUsage usage) {
  m_memoryUsage = usage;
  return *this;
}

ImageBuilder& ImageBuilder::setMemoryFlags(VmaAllocationCreateFlags flags) {
  m_memoryFlags = flags;
  return *this;
}


ImageBuilder& ImageBuilder::setSharingMode(VkSharingMode sharingMode) {
    m_sharingMode = sharingMode;
    return *this;
}

ImageBuilder& ImageBuilder::setQueueFamilyIndices(
    const std::vector<uint32_t>& queueFamilyIndices) {
    m_queueFamilyIndices = queueFamilyIndices;
    return *this;
}

ImageBuilder& ImageBuilder::setInitialLayout(VkImageLayout initialLayout) {
    m_initialLayout = initialLayout;
    return *this;
}

void ImageBuilder::validateParameters() const {
    if (m_format == VK_FORMAT_UNDEFINED) {
        throw std::runtime_error("Image format must be specified");
    }

    if (m_extent.width == 0 || m_extent.height == 0 || m_extent.depth == 0) {
        throw std::runtime_error("Image extent must be greater than 0");
    }

    if (m_usage == 0) {
        throw std::runtime_error("Image usage flags must be specified");
    }

    if (m_sharingMode == VK_SHARING_MODE_CONCURRENT && m_queueFamilyIndices.empty()) {
        throw std::runtime_error("Queue family indices must be specified for concurrent sharing mode");
    }
}

VkImage ImageBuilder::createImage(VmaAllocation* outAllocation) const {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = m_imageType;
    imageInfo.format = m_format;
    imageInfo.extent = m_extent;
    imageInfo.mipLevels = m_mipLevels;
    imageInfo.arrayLayers = m_arrayLayers;
    imageInfo.samples = m_samples;
    imageInfo.tiling = m_tiling;
    imageInfo.usage = m_usage;
    imageInfo.sharingMode = m_sharingMode;
    imageInfo.initialLayout = m_initialLayout;

    if (m_sharingMode == VK_SHARING_MODE_CONCURRENT) {
        imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(m_queueFamilyIndices.size());
        imageInfo.pQueueFamilyIndices = m_queueFamilyIndices.data();
    }

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = m_memoryUsage;
    allocInfo.flags = m_memoryFlags;
    if (m_memoryProperties) {
        allocInfo.requiredFlags = m_memoryProperties;
    }


    VkImage image;
    if (vmaCreateImage(m_device->getAllocator(), &imageInfo, &allocInfo, &image, outAllocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    return image;
}

void ImageBuilder::transitionImageLayout(
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout) const {
    
    auto cmdPool = m_context->getCommandPoolManager();
    VkCommandBuffer cmdBuffer = cmdPool->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = m_mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = m_arrayLayers;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else {
        throw std::runtime_error("unsupported layout transition!");
    }


    vkCmdPipelineBarrier(
        cmdBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    cmdPool->endSingleTimeCommands(cmdBuffer);
}

void ImageBuilder::uploadData(
    ImageInfo imageInfo,
    const void* data,
    VkDeviceSize dataSize,
    VkImageLayout finalImageLayout) const {
    
    // Create staging buffer
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    BufferBuilder stagingBuilder(m_device,m_context);
    stagingBuffer = stagingBuilder
        .setSize(dataSize)
        .setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
        .setMemoryUsage(VMA_MEMORY_USAGE_CPU_ONLY)
        .setMemoryFlags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
        .build("", &stagingAllocation);

    // Copy data to staging buffer
    void* mappedData;
    vmaMapMemory(m_device->getAllocator(), stagingAllocation, &mappedData);
    memcpy(mappedData, data, static_cast<size_t>(dataSize));
    vmaUnmapMemory(m_device->getAllocator(), stagingAllocation);

    // Transition image layout for copy
    ev::ResourceUtils::transitionImageLayoutWithInfo(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool(), imageInfo, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy buffer to image
    auto cmdPool = m_context->getCommandPoolManager();
    VkCommandBuffer cmdBuffer = cmdPool->beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = m_arrayLayers;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = m_extent;

    vkCmdCopyBufferToImage(
        cmdBuffer,
        stagingBuffer,
        imageInfo.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    cmdPool->endSingleTimeCommands(cmdBuffer);

    // Transition image layout for shader access
    ev::ResourceUtils::transitionImageLayoutWithInfo(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool(), imageInfo, finalImageLayout);


    // Cleanup staging buffer
    vmaDestroyBuffer(m_device->getAllocator(), stagingBuffer, stagingAllocation);
}

ImageInfo ImageBuilder::build(
    const std::string& name,
    VmaAllocation* outAllocation) {
    
    VmaAllocation localAllocation;
    outAllocation = &localAllocation;

    validateParameters();

    ImageInfo imageInfo;
    VkImage image = createImage(&imageInfo.allocation);
    VkImageView imageView = createImageView(image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, name);
    
    imageInfo.image = image;
    imageInfo.imageView = imageView;
    imageInfo.width = m_extent.width;
    imageInfo.height = m_extent.height;
    imageInfo.layout = m_initialLayout;

    // Register the image for resource tracking if a name is provided
    if (!name.empty()) {
        m_context->getResourceManager()->registerResource(
            name, reinterpret_cast<uint64_t>(image), imageView, imageInfo.allocation, m_extent.width, m_extent.height, m_initialLayout, VK_OBJECT_TYPE_IMAGE);
    }

    outAllocation = &imageInfo.allocation;

    return imageInfo;
}


ImageInfo ImageBuilder::buildAndInitialize(
    const void* data,
    VkDeviceSize dataSize,
    const std::string& name,
    VmaAllocation* outAllocation,
    VkImageLayout finalImageLayout) {
    
    if (!data || dataSize == 0) {
        throw std::runtime_error("Invalid data or data size");
    }
    

    // Add transfer destination usage flag
    m_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    // Create the image
    ImageInfo imageInfo = build(name, outAllocation);

    // Upload the data
    uploadData(imageInfo, data, dataSize, finalImageLayout);


    return imageInfo;

}

VkImageView ImageBuilder::createImageView(
    VkImage image,
    VkImageViewType viewType,
    VkImageAspectFlags aspectMask,
    const std::string& name) {
    
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = viewType;
    viewInfo.format = m_format;
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = m_mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = m_arrayLayers;

    VkImageView imageView;
    if (vkCreateImageView(m_device->getLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    return imageView;
}

} // namespace ev 