#include "EasyVulkan/Utils/CommandUtils.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include <stdexcept>

namespace ev {
namespace CommandUtils {

VkCommandBuffer beginSingleTimeCommands(
    VulkanDevice* device,
    VkCommandPool pool) {
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(device->getLogicalDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffer!");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin command buffer!");
    }

    return commandBuffer;
}

void endSingleTimeCommands(
    VulkanDevice* device,
    VkCommandPool pool,
    VkCommandBuffer commandBuffer) {
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkFence fence;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (vkCreateFence(device->getLogicalDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    }

    if (vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit command buffer!");
    }

    if (vkWaitForFences(device->getLogicalDevice(), 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
        throw std::runtime_error("failed to wait for fence!");
    }

    vkDestroyFence(device->getLogicalDevice(), fence, nullptr);
    vkFreeCommandBuffers(device->getLogicalDevice(), pool, 1, &commandBuffer);
}


void copyBuffer(
    VulkanDevice* device,
    VkCommandBuffer commandBuffer,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size,
    VkDeviceSize srcOffset,
    VkDeviceSize dstOffset) {
    
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}

void copyBufferToImage(
    VulkanDevice* device,
    VkCommandBuffer commandBuffer,
    VkBuffer srcBuffer,
    VkImage dstImage,
    uint32_t width,
    uint32_t height,
    uint32_t layers) {
    
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layers;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        srcBuffer,
        dstImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
}

void copyImage(
    VulkanDevice* device,
    VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImage dstImage,
    VkImageLayout srcImageLayout,
    VkImageLayout dstImageLayout,
    VkImageAspectFlags srcAspectMask,
    VkImageAspectFlags dstAspectMask,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t baseMipLevel,
    uint32_t levelCount,
    uint32_t baseArrayLayer,
    uint32_t layerCount) {
    
    validateCommandBuffer(commandBuffer);

    // Transition source image to transfer source optimal layout
    if (srcImageLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        VkImageMemoryBarrier srcBarrier{};
        srcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        srcBarrier.oldLayout = srcImageLayout;
        srcBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        srcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrier.image = srcImage;
        srcBarrier.subresourceRange.aspectMask = srcAspectMask;
        srcBarrier.subresourceRange.baseMipLevel = baseMipLevel;
        srcBarrier.subresourceRange.levelCount = levelCount;
        srcBarrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        srcBarrier.subresourceRange.layerCount = layerCount;
        srcBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        srcBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &srcBarrier
        );
    }

    // Transition destination image to transfer destination optimal layout
    if (dstImageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        VkImageMemoryBarrier dstBarrier{};
        dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        dstBarrier.oldLayout = dstImageLayout;
        dstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBarrier.image = dstImage;
        dstBarrier.subresourceRange.aspectMask = dstAspectMask;
        dstBarrier.subresourceRange.baseMipLevel = baseMipLevel;
        dstBarrier.subresourceRange.levelCount = levelCount;
        dstBarrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        dstBarrier.subresourceRange.layerCount = layerCount;
        dstBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        dstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &dstBarrier
        );
    }

    // Perform the copy operation
    VkImageCopy copyRegion{};
    
    // Source subresource
    copyRegion.srcSubresource.aspectMask = srcAspectMask;
    copyRegion.srcSubresource.mipLevel = baseMipLevel;
    copyRegion.srcSubresource.baseArrayLayer = baseArrayLayer;
    copyRegion.srcSubresource.layerCount = layerCount;
    copyRegion.srcOffset = {0, 0, 0};
    
    // Destination subresource
    copyRegion.dstSubresource.aspectMask = dstAspectMask;
    copyRegion.dstSubresource.mipLevel = baseMipLevel;
    copyRegion.dstSubresource.baseArrayLayer = baseArrayLayer;
    copyRegion.dstSubresource.layerCount = layerCount;
    copyRegion.dstOffset = {0, 0, 0};
    
    // Copy extent
    copyRegion.extent = {width, height, depth};

    vkCmdCopyImage(
        commandBuffer,
        srcImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &copyRegion);

    // Transition source image back to original layout
    if (srcImageLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        VkImageMemoryBarrier srcBarrierBack{};
        srcBarrierBack.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        srcBarrierBack.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        srcBarrierBack.newLayout = srcImageLayout;
        srcBarrierBack.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrierBack.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrierBack.image = srcImage;
        srcBarrierBack.subresourceRange.aspectMask = srcAspectMask;
        srcBarrierBack.subresourceRange.baseMipLevel = baseMipLevel;
        srcBarrierBack.subresourceRange.levelCount = levelCount;
        srcBarrierBack.subresourceRange.baseArrayLayer = baseArrayLayer;
        srcBarrierBack.subresourceRange.layerCount = layerCount;
        srcBarrierBack.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        srcBarrierBack.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &srcBarrierBack
        );
    }

    // Transition destination image back to original layout
    if (dstImageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        VkImageMemoryBarrier dstBarrierBack{};
        dstBarrierBack.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        dstBarrierBack.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        dstBarrierBack.newLayout = dstImageLayout;
        dstBarrierBack.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBarrierBack.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBarrierBack.image = dstImage;
        dstBarrierBack.subresourceRange.aspectMask = dstAspectMask;
        dstBarrierBack.subresourceRange.baseMipLevel = baseMipLevel;
        dstBarrierBack.subresourceRange.levelCount = levelCount;
        dstBarrierBack.subresourceRange.baseArrayLayer = baseArrayLayer;
        dstBarrierBack.subresourceRange.layerCount = layerCount;
        dstBarrierBack.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstBarrierBack.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &dstBarrierBack
        );
    }
}

void validateCommandBuffer(VkCommandBuffer commandBuffer) {
    if (commandBuffer == VK_NULL_HANDLE) {
        throw std::runtime_error("Command buffer recording not started");
    }
}

void beginCommandBuffer(
    VkCommandBuffer commandBuffer,
    VkCommandBufferUsageFlags flags) {
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void endCommandBuffer(VkCommandBuffer commandBuffer) {
    validateCommandBuffer(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void bindPipeline(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipeline pipeline) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

void bindDescriptorSets(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t firstSet,
    const std::vector<VkDescriptorSet>& descriptorSets,
    const std::vector<uint32_t>& dynamicOffsets) {
    
    validateCommandBuffer(commandBuffer);

    vkCmdBindDescriptorSets(
        commandBuffer,
        pipelineBindPoint,
        layout,
        firstSet,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(),
        static_cast<uint32_t>(dynamicOffsets.size()),
        dynamicOffsets.data());
}

void bindVertexBuffers(
    VkCommandBuffer commandBuffer,
    uint32_t firstBinding,
    const std::vector<VkBuffer>& buffers,
    const std::vector<VkDeviceSize>& offsets) {
    
    validateCommandBuffer(commandBuffer);

    if (buffers.size() != offsets.size()) {
        throw std::runtime_error("Number of buffers must match number of offsets");
    }

    vkCmdBindVertexBuffers(
        commandBuffer,
        firstBinding,
        static_cast<uint32_t>(buffers.size()),
        buffers.data(),
        offsets.data());
}

void bindIndexBuffer(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkIndexType indexType) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void draw(
    VkCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void drawIndexed(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void beginRenderPass(
    VkCommandBuffer commandBuffer,
    const VkRenderPassBeginInfo& renderPassBegin,
    VkSubpassContents contents) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, contents);
}

void endRenderPass(VkCommandBuffer commandBuffer) {
    validateCommandBuffer(commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
}

void setViewport(
    VkCommandBuffer commandBuffer,
    uint32_t firstViewport,
    const std::vector<VkViewport>& viewports) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdSetViewport(commandBuffer, firstViewport, static_cast<uint32_t>(viewports.size()), viewports.data());
}

void setScissor(
    VkCommandBuffer commandBuffer,
    uint32_t firstScissor,
    const std::vector<VkRect2D>& scissors) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdSetScissor(commandBuffer, firstScissor, static_cast<uint32_t>(scissors.size()), scissors.data());
}

void pushConstants(
    VkCommandBuffer commandBuffer,
    VkPipelineLayout layout,
    VkShaderStageFlags stageFlags,
    uint32_t offset,
    uint32_t size,
    const void* pValues) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}



void clearColorImage(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout imageLayout,
    const VkClearColorValue& color,
    const std::vector<VkImageSubresourceRange>& ranges) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdClearColorImage(
        commandBuffer,
        image,
        imageLayout,
        &color,
        static_cast<uint32_t>(ranges.size()),
        ranges.data());
}

void clearDepthStencilImage(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout imageLayout,
    const VkClearDepthStencilValue& depthStencil,
    const std::vector<VkImageSubresourceRange>& ranges) {
    
    validateCommandBuffer(commandBuffer);
    vkCmdClearDepthStencilImage(
        commandBuffer,
        image,
        imageLayout,
        &depthStencil,
        static_cast<uint32_t>(ranges.size()),
        ranges.data());
}

void pipelineBarrier(
    VkCommandBuffer commandBuffer,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    const std::vector<VkMemoryBarrier>& memoryBarriers,
    const std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers,
    const std::vector<VkImageMemoryBarrier>& imageMemoryBarriers) {
    
    validateCommandBuffer(commandBuffer);

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask,
        dstStageMask,
        dependencyFlags,
        static_cast<uint32_t>(memoryBarriers.size()),
        memoryBarriers.data(),
        static_cast<uint32_t>(bufferMemoryBarriers.size()),
        bufferMemoryBarriers.data(),
        static_cast<uint32_t>(imageMemoryBarriers.size()),
        imageMemoryBarriers.data());
}

void computeToComputeImageBarrier(
    VkCommandBuffer cmdBuffer,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkAccessFlags srcAccess,
    VkAccessFlags dstAccess,
    VkImageAspectFlags aspectMask,
    uint32_t baseMipLevel,
    uint32_t levelCount,
    uint32_t baseArrayLayer,
    uint32_t layerCount,
    uint32_t srcQueueFamily,
    uint32_t dstQueueFamily

) {
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = srcQueueFamily;
    barrier.dstQueueFamilyIndex = dstQueueFamily;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = baseMipLevel;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
    barrier.subresourceRange.layerCount = layerCount;
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;

    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // srcStage
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,  // dstStage
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void computeToGfxImageBarrier(
    VkCommandBuffer      cmdBuffer,
    VkImage              image,
    VkImageLayout        oldLayout,
    VkImageLayout        newLayout,
    VkAccessFlags        srcAccess,
    VkAccessFlags        dstAccess,
    VkImageAspectFlags   aspectMask,
    uint32_t             baseMipLevel,
    uint32_t             levelCount,
    uint32_t             baseArrayLayer,
    uint32_t             layerCount,
    uint32_t             srcQueueFamily,
    uint32_t             dstQueueFamily
) {
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
    // No layout transition here: the same layout is used for both old and new.
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    
    barrier.srcQueueFamilyIndex = srcQueueFamily;
    barrier.dstQueueFamilyIndex = dstQueueFamily;
    barrier.image = image;
    
    barrier.subresourceRange.aspectMask     = aspectMask;
    barrier.subresourceRange.baseMipLevel     = baseMipLevel;
    barrier.subresourceRange.levelCount       = levelCount;
    barrier.subresourceRange.baseArrayLayer   = baseArrayLayer;
    barrier.subresourceRange.layerCount       = layerCount;
    
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;

    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Source stage: compute shader writes
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,      // No memory barriers
        nullptr,
        0,      // No buffer memory barriers
        nullptr,
        1,      // One image memory barrier
        &barrier
    );
}

void gfxToComputeImageBarrier(
    VkCommandBuffer      cmdBuffer,
    VkImage              image,
    VkImageLayout        oldLayout,
    VkImageLayout        newLayout,
    VkAccessFlags        srcAccess,
    VkAccessFlags        dstAccess,
    VkImageAspectFlags   aspectMask,
    uint32_t             baseMipLevel,
    uint32_t             levelCount,
    uint32_t             baseArrayLayer,
    uint32_t             layerCount,
    uint32_t             srcQueueFamily,
    uint32_t             dstQueueFamily
) {
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    // No layout transition: both old and new layouts are the same.
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    // Set queue family indices in case of ownership transfer.
    barrier.srcQueueFamilyIndex = srcQueueFamily;
    barrier.dstQueueFamilyIndex = dstQueueFamily;

    barrier.image = image;
    barrier.subresourceRange.aspectMask     = aspectMask;
    barrier.subresourceRange.baseMipLevel     = baseMipLevel;
    barrier.subresourceRange.levelCount       = levelCount;
    barrier.subresourceRange.baseArrayLayer   = baseArrayLayer;
    barrier.subresourceRange.layerCount       = layerCount;

    // Set access masks for proper synchronization.
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;

    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,   // Destination stage: compute shader stage
        0,                                      // No dependency flags
        0, nullptr,                           // No memory barriers
        0, nullptr,                           // No buffer memory barriers
        1, &barrier                           // One image memory barrier
    );
}

void gfxToGfxImageBarrier(
    VkCommandBuffer      cmdBuffer,
    VkImage              image,
    VkImageLayout        oldLayout,
    VkImageLayout        newLayout,
    VkAccessFlags        srcAccess,
    VkAccessFlags        dstAccess,
    VkImageAspectFlags   aspectMask,
    uint32_t             baseMipLevel,
    uint32_t             levelCount,
    uint32_t             baseArrayLayer,
    uint32_t             layerCount,
    uint32_t             srcQueueFamily,
    uint32_t             dstQueueFamily
) {
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
    // Set the layout transition
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    
    // Set queue family indices in case of ownership transfer
    barrier.srcQueueFamilyIndex = srcQueueFamily;
    barrier.dstQueueFamilyIndex = dstQueueFamily;
    
    // Set image and subresource range
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = baseMipLevel;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
    barrier.subresourceRange.layerCount = layerCount;
    
    // Set access masks for proper synchronization
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;
    
    vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // Source stage: color attachment output
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,          // Destination stage: fragment shader
        0,                                              // No dependency flags
        0, nullptr,                                     // No memory barriers
        0, nullptr,                                     // No buffer memory barriers
        1, &barrier                                     // One image memory barrier
    );
}

} // namespace CommandUtils
} // namespace ev 