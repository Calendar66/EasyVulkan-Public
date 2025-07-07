#include "EasyVulkan/Core/ResourceManager.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Builders/BufferBuilder.hpp"
#include "EasyVulkan/Builders/ImageBuilder.hpp"
#include "EasyVulkan/Builders/GraphicsPipelineBuilder.hpp"
#include "EasyVulkan/Builders/ComputePipelineBuilder.hpp"
#include "EasyVulkan/Builders/DescriptorSetBuilder.hpp"
#include "EasyVulkan/Builders/RenderPassBuilder.hpp"
#include "EasyVulkan/Builders/FramebufferBuilder.hpp"
#include "EasyVulkan/Builders/CommandBufferBuilder.hpp"
#include "EasyVulkan/Builders/SamplerBuilder.hpp"
#include "EasyVulkan/Builders/ShaderModuleBuilder.hpp"
#include "EasyVulkan/Utils/CommandUtils.hpp"
#include "EasyVulkan/Core/CommandPoolManager.hpp"
#include "EasyVulkan/Utils/VulkanDebug.hpp"
#include <stdexcept>

namespace ev {

ResourceManager::ResourceManager(VulkanDevice* device, VulkanContext* context)
    : m_device(device)
    , m_context(context) {
}

ResourceManager::~ResourceManager() {
    cleanup();
}

BufferBuilder ResourceManager::createBuffer() {
    return BufferBuilder(m_device,m_context);
}

ImageBuilder ResourceManager::createImage() {
    return ImageBuilder(m_device,m_context);
}

GraphicsPipelineBuilder ResourceManager::createGraphicsPipeline() {
    return GraphicsPipelineBuilder(m_device,m_context);
}

ComputePipelineBuilder ResourceManager::createComputePipeline() {
    return ComputePipelineBuilder(m_device,m_context);
}

DescriptorSetBuilder ResourceManager::createDescriptorSet() {
    return DescriptorSetBuilder(m_device,m_context);
}

RenderPassBuilder ResourceManager::createRenderPass() {
    return RenderPassBuilder(m_device,m_context);
}

FramebufferBuilder ResourceManager::createFramebuffer() {
    return FramebufferBuilder(m_device,m_context);
}

CommandBufferBuilder ResourceManager::createCommandBuffer() {
    return CommandBufferBuilder(m_device,m_context);
}

SamplerBuilder ResourceManager::createSampler() {
    return SamplerBuilder(m_device,m_context);
}

ShaderModuleBuilder ResourceManager::createShaderModule() {
    return ShaderModuleBuilder(m_device,m_context);
}

void ResourceManager::registerResource(const std::string& name, uint64_t handle, VkObjectType type) {
    if (name.empty()) {
        return;
    }

    switch (type) {
        case VK_OBJECT_TYPE_RENDER_PASS:
            m_renderPasses[name] = reinterpret_cast<VkRenderPass>(handle);
            break;
        case VK_OBJECT_TYPE_FRAMEBUFFER:
            m_framebuffers[name] = reinterpret_cast<VkFramebuffer>(handle);
            break;
        case VK_OBJECT_TYPE_SAMPLER:
            m_samplers[name] = reinterpret_cast<VkSampler>(handle);
            break;
        case VK_OBJECT_TYPE_SHADER_MODULE:
            m_shaderModules[name] = reinterpret_cast<VkShaderModule>(handle);
            break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
            m_descriptorSetLayouts[name] = reinterpret_cast<VkDescriptorSetLayout>(handle);
            break;
        default:
            LogError("This kind of resource tracking should be done with this overload of registerResource(Supported types: RenderPass, Framebuffer, Sampler, ShaderModule)");
            throw std::runtime_error("Unsupported resource type for tracking(For RenderPass, Framebuffer, Sampler, ShaderModule)");
    }

    // Set debug name for the resource
    if (!name.empty()) {
        ev::VulkanDebug::setDebugObjectName(m_device->getLogicalDevice(),type, (uint64_t)handle, name);
    }
}

void ResourceManager::registerResource(const std::string& name, uint64_t handle,
                                VmaAllocation allocation, VkDeviceSize size, 
                                VkBufferUsageFlags usage, VkObjectType type) {
    if (name.empty()) {
        return;
    }

    if (type == VK_OBJECT_TYPE_BUFFER) {
        BufferInfo bufferInfo;
        bufferInfo.buffer = reinterpret_cast<VkBuffer>(handle);
        bufferInfo.allocation = allocation;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        m_buffers[name] = bufferInfo;
    } else {
        LogError("This kind of resource tracking should be done with this overload of registerResource(Supported types: Buffer)");
        throw std::runtime_error("Unsupported resource type for VMA tracking(For Buffer)");
    }

    // Set debug name for the resource
    if (!name.empty()) {
        ev::VulkanDebug::setDebugObjectName(m_device->getLogicalDevice(),type, (uint64_t)handle, name);
    }
}

void ResourceManager::registerResource(const std::string& name, uint64_t handle,
    VkImageView imageView, VmaAllocation allocation,  uint32_t width, uint32_t height, VkImageLayout layout, VkObjectType type) {
    if (name.empty()) {
        return;
    }
    switch (type) {
        case VK_OBJECT_TYPE_IMAGE:
            ImageInfo imageInfo;
            imageInfo.image = reinterpret_cast<VkImage>(handle);
            imageInfo.imageView = imageView;
            imageInfo.allocation = allocation;
            imageInfo.width = width;
            imageInfo.height = height;
            imageInfo.layout = layout;
            m_images[name] = imageInfo;
            break;
        default:
            LogError("This kind of resource tracking should be done with this overload of registerResource(Supported types: Image)");
            throw std::runtime_error("Unsupported resource type for VMA tracking(For Image)");
    }

    // Set debug name for the resource
    if (!name.empty()) {
        ev::VulkanDebug::setDebugObjectName(m_device->getLogicalDevice(),type, (uint64_t)handle, name);
    }
}



void ResourceManager::registerResource(const std::string& name, uint64_t primaryHandle,
                                uint64_t secondaryHandle, VkObjectType type) {
    if (name.empty()) {
        return;
    }
    switch (type) {
        case VK_OBJECT_TYPE_PIPELINE:
            PipelineInfo pipelineInfo;
            pipelineInfo.pipeline = reinterpret_cast<VkPipeline>(primaryHandle);
            pipelineInfo.pipelineLayout = reinterpret_cast<VkPipelineLayout>(secondaryHandle);
            m_pipelines[name] = pipelineInfo;
            break;
        case VK_OBJECT_TYPE_COMMAND_BUFFER:
            CommandBufferInfo commandBufferInfo;
            commandBufferInfo.commandBuffer = reinterpret_cast<VkCommandBuffer>(primaryHandle);
            commandBufferInfo.commandPool = reinterpret_cast<VkCommandPool>(secondaryHandle);
            m_commandBuffers[name] = commandBufferInfo;
            break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:
            DescriptorSetInfo descriptorSetInfo;
            descriptorSetInfo.descriptorSet = reinterpret_cast<VkDescriptorSet>(primaryHandle);
            descriptorSetInfo.descriptorPool = reinterpret_cast<VkDescriptorPool>(secondaryHandle);
            m_descriptorSetInfos[name] = descriptorSetInfo;
            break;
        default:
            LogError("This kind of resource tracking should be done with this overload of registerResource(Supported types: Pipeline, DescriptorSet, CommandBuffer)");
            throw std::runtime_error("Unsupported resource type for tracking(For Pipeline, DescriptorSet, CommandBuffer)");
    }

    // Set debug name for the resource
    if (!name.empty()) {
        ev::VulkanDebug::setDebugObjectName(m_device->getLogicalDevice(),type, (uint64_t)primaryHandle, name);
    }
}


bool ResourceManager::clearResource(const std::string& name, VkObjectType type) {
    if (name.empty()) {
        return false;
    }

    bool found = false;
    switch (type) {
        // Single-handle resources
        case VK_OBJECT_TYPE_BUFFER:
            if (m_buffers.find(name) != m_buffers.end()) {
                vmaDestroyBuffer(m_device->getAllocator(), m_buffers[name].buffer, m_buffers[name].allocation);
                m_buffers.erase(name);
                found = true;
            }
            break;
        case VK_OBJECT_TYPE_RENDER_PASS:
            if (m_renderPasses.find(name) != m_renderPasses.end()) {
                vkDestroyRenderPass(m_device->getLogicalDevice(), m_renderPasses[name], nullptr);
                m_renderPasses.erase(name);
                found = true;
            }
            break;
        case VK_OBJECT_TYPE_FRAMEBUFFER:
            if (m_framebuffers.find(name) != m_framebuffers.end()) {
                vkDestroyFramebuffer(m_device->getLogicalDevice(), m_framebuffers[name], nullptr);
                m_framebuffers.erase(name);
                found = true;
            }
            break;
        case VK_OBJECT_TYPE_SAMPLER:
            if (m_samplers.find(name) != m_samplers.end()) {
                vkDestroySampler(m_device->getLogicalDevice(), m_samplers[name], nullptr);
                m_samplers.erase(name);
                found = true;
            }
            break;
        case VK_OBJECT_TYPE_SHADER_MODULE:
            if (m_shaderModules.find(name) != m_shaderModules.end()) {
                vkDestroyShaderModule(m_device->getLogicalDevice(), m_shaderModules[name], nullptr);
                m_shaderModules.erase(name);
                found = true;
            }
            break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
            if (m_descriptorSetLayouts.find(name) != m_descriptorSetLayouts.end()) {
                vkDestroyDescriptorSetLayout(m_device->getLogicalDevice(), m_descriptorSetLayouts[name], nullptr);
                m_descriptorSetLayouts.erase(name);
                found = true;
            }
            break;
        case VK_OBJECT_TYPE_PIPELINE:
            if (m_pipelines.find(name) != m_pipelines.end()) {
                vkDestroyPipeline(m_device->getLogicalDevice(), m_pipelines[name].pipeline, nullptr);
                vkDestroyPipelineLayout(m_device->getLogicalDevice(), m_pipelines[name].pipelineLayout, nullptr);
                m_pipelines.erase(name);
                found = true;
            }
            break;
        case VK_OBJECT_TYPE_COMMAND_BUFFER:
            if (m_commandBuffers.find(name) != m_commandBuffers.end()) {
                vkFreeCommandBuffers(m_device->getLogicalDevice(), m_commandBuffers[name].commandPool, 1, &m_commandBuffers[name].commandBuffer);
                m_commandBuffers.erase(name);
                found = true;
            }
            break;
        case VK_OBJECT_TYPE_IMAGE:
            if (m_images.find(name) != m_images.end()) {
                vkDestroyImageView(m_device->getLogicalDevice(), m_images[name].imageView, nullptr);
                vmaDestroyImage(m_device->getAllocator(), m_images[name].image, m_images[name].allocation);
                m_images.erase(name);
                found = true;
            }
            break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:
            if (m_descriptorSetInfos.find(name) != m_descriptorSetInfos.end()) {
                // First, free the descriptor set
                vkFreeDescriptorSets(m_device->getLogicalDevice(), m_descriptorSetInfos[name].descriptorPool, 1, &m_descriptorSetInfos[name].descriptorSet);
                // Then, free the descriptor pool
                vkDestroyDescriptorPool(m_device->getLogicalDevice(), m_descriptorSetInfos[name].descriptorPool, nullptr);
                // Finally, remove the descriptor set info from the map
                m_descriptorSetInfos.erase(name);
                found = true;
            }else{
                LogError("Descriptor set not found for clearing: " + name);
            }
            break;
        default:
            LogError("Unsupported resource type for clearing");
            break;
    }
    
    return found;
}

void ResourceManager::cleanup() {
    VkDevice device = m_device->getLogicalDevice();

    // Cleanup all tracked resources in reverse order of typical dependencies
    for (const auto& pair : m_framebuffers) {
        vkDestroyFramebuffer(device, pair.second, nullptr);
    }
    m_framebuffers.clear();

    for (const auto& pair : m_renderPasses) {
        vkDestroyRenderPass(device, pair.second, nullptr);
    }
    m_renderPasses.clear();

    for (const auto& pair : m_pipelines) {
        vkDestroyPipeline(device, pair.second.pipeline, nullptr);
        vkDestroyPipelineLayout(device, pair.second.pipelineLayout, nullptr);
    }
    m_pipelines.clear();

    for (const auto& pair : m_shaderModules) {
        vkDestroyShaderModule(device, pair.second, nullptr);
    }
    m_shaderModules.clear();

    for (const auto& pair : m_samplers) {
        vkDestroySampler(device, pair.second, nullptr);
    }
    m_samplers.clear();

    for (const auto& pair : m_images) {
        vkDestroyImageView(device, pair.second.imageView, nullptr);
        vmaDestroyImage(m_device->getAllocator(), pair.second.image, pair.second.allocation);
    }
    m_images.clear();

    for (const auto& pair : m_buffers) {
        vmaDestroyBuffer(m_device->getAllocator(), pair.second.buffer, pair.second.allocation);
    }
    m_buffers.clear();

    // Free command buffers with their associated pools
    for (const auto& pair : m_commandBuffers) {
        const auto& info = pair.second;
        if (info.commandPool != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(device, info.commandPool, 1, &info.commandBuffer);
        }
    }
    m_commandBuffers.clear();

    // Free descriptor sets with their associated pools
    for (const auto& pair : m_descriptorSetInfos) {
        vkFreeDescriptorSets(device, pair.second.descriptorPool, 1, &pair.second.descriptorSet);
    }

    // After that，we should free the descriptor pool
    for (const auto& pair : m_descriptorSetInfos) {
        vkDestroyDescriptorPool(device, pair.second.descriptorPool, nullptr);
    }
    m_descriptorSetInfos.clear();
}

void ResourceManager::destroyResource(uint64_t handle, VkObjectType type) {
    VkDevice device = m_device->getLogicalDevice();

    switch (type) {
        case VK_OBJECT_TYPE_BUFFER:
            vkDestroyBuffer(device, reinterpret_cast<VkBuffer>(handle), nullptr);
            break;
        case VK_OBJECT_TYPE_IMAGE:
            vkDestroyImage(device, reinterpret_cast<VkImage>(handle), nullptr);
            break;
        case VK_OBJECT_TYPE_PIPELINE:
            vkDestroyPipeline(device, reinterpret_cast<VkPipeline>(handle), nullptr);
            break;
        case VK_OBJECT_TYPE_RENDER_PASS:
            vkDestroyRenderPass(device, reinterpret_cast<VkRenderPass>(handle), nullptr);
            break;
        case VK_OBJECT_TYPE_FRAMEBUFFER:
            vkDestroyFramebuffer(device, reinterpret_cast<VkFramebuffer>(handle), nullptr);
            break;
        case VK_OBJECT_TYPE_SAMPLER:
            vkDestroySampler(device, reinterpret_cast<VkSampler>(handle), nullptr);
            break;
        case VK_OBJECT_TYPE_SHADER_MODULE:
            vkDestroyShaderModule(device, reinterpret_cast<VkShaderModule>(handle), nullptr);
            break;
        default:
            throw std::runtime_error("Unsupported resource type for destruction");
    }
}

std::vector<VmaBudget> ResourceManager::getMemoryBudget() const {
    VmaAllocator allocator = m_device->getAllocator();
    if (!allocator) {
        throw std::runtime_error("VMA allocator not initialized");
    }

    // Get memory properties to determine the number of heaps
    const VkPhysicalDeviceMemoryProperties* memProps;
    vmaGetMemoryProperties(allocator, &memProps);
    uint32_t heapCount = memProps->memoryHeapCount;

    // Allocate and fill the budget array
    std::vector<VmaBudget> budgets(heapCount);
    vmaGetHeapBudgets(allocator, budgets.data());

    return budgets;
}

VmaTotalStatistics ResourceManager::getMemoryUsage() const {
    VmaAllocator allocator = m_device->getAllocator();
    if (!allocator) {
        throw std::runtime_error("VMA allocator not initialized");
    }

    VmaTotalStatistics stats;
    vmaCalculateStatistics(allocator, &stats);

    return stats;
}

VmaDefragmentationStats ResourceManager::defragmentMemory(VkDeviceSize maxBytesPerPass, uint32_t maxAllocationsPerPass) {
    VmaAllocator allocator = m_device->getAllocator();
    if (!allocator) {
        throw std::runtime_error("VMA allocator not initialized");
    }

    // Setup defragmentation info
    VmaDefragmentationInfo defragInfo = {};
    defragInfo.maxBytesPerPass = maxBytesPerPass;
    defragInfo.maxAllocationsPerPass = maxAllocationsPerPass;
    defragInfo.pool = VK_NULL_HANDLE; // Default pools

    // Begin defragmentation
    VmaDefragmentationContext context;
    VkResult result = vmaBeginDefragmentation(allocator, &defragInfo, &context);
    
    if (result != VK_SUCCESS) {
        if (result == VK_ERROR_FEATURE_NOT_PRESENT) {
            throw std::runtime_error("Memory defragmentation is not supported on this device");
        } else {
            throw std::runtime_error("Failed to begin memory defragmentation");
        }
    }

    // Process defragmentation passes
    VmaDefragmentationStats stats = {};
    
    while (true) {
        VmaDefragmentationPassMoveInfo passInfo = {};
        result = vmaBeginDefragmentationPass(allocator, context, &passInfo);
        
        if (result == VK_SUCCESS) {
            // No more moves needed, we're done
            break;
        } else if (result == VK_INCOMPLETE) {
            // Process the moves
            for (uint32_t i = 0; i < passInfo.moveCount; ++i) {
                VmaDefragmentationMove& move = passInfo.pMoves[i];
                
                // Get allocation info for the source allocation
                VmaAllocationInfo allocInfo;
                vmaGetAllocationInfo(allocator, move.srcAllocation, &allocInfo);
                
                // Create a temporary buffer to hold the data during the move
                VkBufferCreateInfo bufferInfo = {};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = allocInfo.size;
                bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                
                VkBuffer tempBuffer;
                VmaAllocation tempAllocation;
                
                VmaAllocationCreateInfo allocCreateInfo = {};
                allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                
                if (vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &tempBuffer, &tempAllocation, nullptr) != VK_SUCCESS) {
                    // If we can't create a temporary buffer, skip this move
                    move.operation = VMA_DEFRAGMENTATION_MOVE_OPERATION_IGNORE;
                    continue;
                }
                
                // Copy data from source to temp buffer
                VkCommandBuffer cmdBuffer = ev::CommandUtils::beginSingleTimeCommands(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool());
                
                VkBufferCopy copyRegion = {};
                copyRegion.size = allocInfo.size;
                vkCmdCopyBuffer(cmdBuffer, reinterpret_cast<VkBuffer>(allocInfo.pUserData), tempBuffer, 1, &copyRegion);
                
                ev::CommandUtils::endSingleTimeCommands(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool(), cmdBuffer);
                
                // Copy data from temp to destination
                cmdBuffer = ev::CommandUtils::beginSingleTimeCommands(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool());
                
                vkCmdCopyBuffer(cmdBuffer, tempBuffer, reinterpret_cast<VkBuffer>(move.dstTmpAllocation), 1, &copyRegion);
                
                ev::CommandUtils::endSingleTimeCommands(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool(), cmdBuffer);
                
                // Clean up temp buffer
                vmaDestroyBuffer(allocator, tempBuffer, tempAllocation);
                
                // Mark the move as completed
                move.operation = VMA_DEFRAGMENTATION_MOVE_OPERATION_COPY;
            }
            
            // End the pass
            vmaEndDefragmentationPass(allocator, context, &passInfo);
        } else {
            // Error occurred
            vmaEndDefragmentation(allocator, context, &stats);
            throw std::runtime_error("Error during defragmentation pass");
        }
    }
    
    // End defragmentation and get stats
    vmaEndDefragmentation(allocator, context, &stats);
    
    return stats;
}

VmaDefragmentationStats ResourceManager::defragmentMemoryPool(VmaPool pool, VkDeviceSize maxBytesPerPass, uint32_t maxAllocationsPerPass) {
    VmaAllocator allocator = m_device->getAllocator();
    if (!allocator) {
        throw std::runtime_error("VMA allocator not initialized");
    }
    
    if (!pool) {
        throw std::runtime_error("Invalid memory pool handle");
    }

    // Setup defragmentation info
    VmaDefragmentationInfo defragInfo = {};
    defragInfo.maxBytesPerPass = maxBytesPerPass;
    defragInfo.maxAllocationsPerPass = maxAllocationsPerPass;
    defragInfo.pool = pool;

    // Begin defragmentation
    VmaDefragmentationContext context;
    VkResult result = vmaBeginDefragmentation(allocator, &defragInfo, &context);
    
    if (result != VK_SUCCESS) {
        if (result == VK_ERROR_FEATURE_NOT_PRESENT) {
            throw std::runtime_error("Memory defragmentation is not supported for this pool");
        } else {
            throw std::runtime_error("Failed to begin memory defragmentation");
        }
    }

    // Process defragmentation passes
    VmaDefragmentationStats stats = {};
    
    while (true) {
        VmaDefragmentationPassMoveInfo passInfo = {};
        result = vmaBeginDefragmentationPass(allocator, context, &passInfo);
        
        if (result == VK_SUCCESS) {
            // No more moves needed, we're done
            break;
        } else if (result == VK_INCOMPLETE) {
            // Process the moves
            for (uint32_t i = 0; i < passInfo.moveCount; ++i) {
                VmaDefragmentationMove& move = passInfo.pMoves[i];
                
                // Get allocation info for the source allocation
                VmaAllocationInfo allocInfo;
                vmaGetAllocationInfo(allocator, move.srcAllocation, &allocInfo);
                
                // Create a temporary buffer to hold the data during the move
                VkBufferCreateInfo bufferInfo = {};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = allocInfo.size;
                bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                
                VkBuffer tempBuffer;
                VmaAllocation tempAllocation;
                
                VmaAllocationCreateInfo allocCreateInfo = {};
                allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                
                if (vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &tempBuffer, &tempAllocation, nullptr) != VK_SUCCESS) {
                    // If we can't create a temporary buffer, skip this move
                    move.operation = VMA_DEFRAGMENTATION_MOVE_OPERATION_IGNORE;
                    continue;
                }
                
                // Copy data from source to temp buffer
                VkCommandBuffer cmdBuffer = ev::CommandUtils::beginSingleTimeCommands(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool());
                
                VkBufferCopy copyRegion = {};
                copyRegion.size = allocInfo.size;
                vkCmdCopyBuffer(cmdBuffer, reinterpret_cast<VkBuffer>(allocInfo.pUserData), tempBuffer, 1, &copyRegion);
                
                ev::CommandUtils::endSingleTimeCommands(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool(), cmdBuffer);
                
                // Copy data from temp to destination
                cmdBuffer = ev::CommandUtils::beginSingleTimeCommands(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool());
                
                vkCmdCopyBuffer(cmdBuffer, tempBuffer, reinterpret_cast<VkBuffer>(move.dstTmpAllocation), 1, &copyRegion);
                
                ev::CommandUtils::endSingleTimeCommands(m_device, m_context->getCommandPoolManager()->getSingleTimeCommandPool(), cmdBuffer);
                
                // Clean up temp buffer
                vmaDestroyBuffer(allocator, tempBuffer, tempAllocation);
                
                // Mark the move as completed
                move.operation = VMA_DEFRAGMENTATION_MOVE_OPERATION_COPY;
            }
            
            // End the pass
            vmaEndDefragmentationPass(allocator, context, &passInfo);
        } else {
            // Error occurred
            vmaEndDefragmentation(allocator, context, &stats);
            throw std::runtime_error("Error during defragmentation pass");
        }
    }
    
    // End defragmentation and get stats
    vmaEndDefragmentation(allocator, context, &stats);
    
    return stats;
}

void ResourceManager::printMemoryUsage(bool detailed) const {
    try {
        // Get memory budget information
        std::vector<VmaBudget> budgets = getMemoryBudget();
        
        // Get memory properties to determine heap types
        VmaAllocator allocator = m_device->getAllocator();
        const VkPhysicalDeviceMemoryProperties* memProps;
        vmaGetMemoryProperties(allocator, &memProps);
        
        // Print summary for each heap
        printf("\n===== MEMORY USAGE SUMMARY =====\n");
        for (size_t i = 0; i < budgets.size(); ++i) {
            const VmaBudget& budget = budgets[i];
            const VkMemoryHeap& heap = memProps->memoryHeaps[i];
            
            // Determine if this is device local (GPU) memory
            bool isDeviceLocal = (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0;
            const char* heapType = isDeviceLocal ? "GPU" : "CPU";
            
            // Calculate usage percentage
            float usagePercent = (budget.budget > 0) 
                ? (float)budget.usage * 100.0f / (float)budget.budget 
                : 0.0f;
            
            // Print heap summary
            printf("Heap %zu (%s): %0.1f MB used / %0.1f MB budget (%0.1f%%)\n",
                   i, heapType,
                   (double)budget.usage / (1024.0 * 1024.0),
                   (double)budget.budget / (1024.0 * 1024.0),
                   usagePercent);
            
            // Print allocation statistics
            printf("  Allocations: %u, Blocks: %u\n",
                   budget.statistics.allocationCount,
                   budget.statistics.blockCount);
            
            // Print memory details
            printf("  Allocated: %0.1f MB, Unused: %0.1f MB\n",
                   (double)budget.statistics.allocationBytes / (1024.0 * 1024.0),
                   (double)(budget.statistics.blockBytes - budget.statistics.allocationBytes) / (1024.0 * 1024.0));
        }
        
        // If detailed statistics are requested, print them
        if (detailed) {
            VmaTotalStatistics stats = getMemoryUsage();
            
            printf("\n===== DETAILED MEMORY STATISTICS =====\n");
            printf("Total Statistics:\n");
            printf("  Allocations: %u\n", stats.total.statistics.allocationCount);
            printf("  Blocks: %u\n", stats.total.statistics.blockCount);
            printf("  Allocated: %0.1f MB\n", (double)stats.total.statistics.allocationBytes / (1024.0 * 1024.0));
            printf("  Block memory: %0.1f MB\n", (double)stats.total.statistics.blockBytes / (1024.0 * 1024.0));
            printf("  Unused range count: %u\n", stats.total.unusedRangeCount);
            
            if (stats.total.unusedRangeCount > 0) {
                printf("  Unused range size: min=%0.1f KB, avg=%0.1f KB, max=%0.1f KB\n",
                       (double)stats.total.unusedRangeSizeMin / 1024.0,
                       stats.total.unusedRangeCount > 0 
                           ? (double)stats.total.statistics.blockBytes - stats.total.statistics.allocationBytes / (double)stats.total.unusedRangeCount / 1024.0 
                           : 0.0,
                       (double)stats.total.unusedRangeSizeMax / 1024.0);
            }
            
            // Print per-heap detailed statistics
            printf("\nPer-Heap Statistics:\n");
            for (uint32_t i = 0; i < memProps->memoryHeapCount; ++i) {
                const VkMemoryHeap& heap = memProps->memoryHeaps[i];
                const VmaDetailedStatistics& heapStats = stats.memoryHeap[i];
                
                // Determine if this is device local (GPU) memory
                bool isDeviceLocal = (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0;
                const char* heapType = isDeviceLocal ? "GPU" : "CPU";
                
                printf("Heap %u (%s):\n", i, heapType);
                printf("  Allocations: %u\n", heapStats.statistics.allocationCount);
                printf("  Blocks: %u\n", heapStats.statistics.blockCount);
                printf("  Allocated: %0.1f MB\n", (double)heapStats.statistics.allocationBytes / (1024.0 * 1024.0));
                printf("  Block memory: %0.1f MB\n", (double)heapStats.statistics.blockBytes / (1024.0 * 1024.0));
                printf("  Unused range count: %u\n", heapStats.unusedRangeCount);
                
                if (heapStats.unusedRangeCount > 0) {
                    printf("  Unused range size: min=%0.1f KB, avg=%0.1f KB, max=%0.1f KB\n",
                           (double)heapStats.unusedRangeSizeMin / 1024.0,
                           heapStats.unusedRangeCount > 0 
                               ? (double)(heapStats.statistics.blockBytes - heapStats.statistics.allocationBytes) / (double)heapStats.unusedRangeCount / 1024.0 
                               : 0.0,
                           (double)heapStats.unusedRangeSizeMax / 1024.0);
                }
                
                // Print memory type statistics for this heap
                printf("  Memory Types:\n");
                for (uint32_t j = 0; j < memProps->memoryTypeCount; ++j) {
                    if (memProps->memoryTypes[j].heapIndex == i) {
                        const VmaDetailedStatistics& typeStats = stats.memoryType[j];
                        
                        // Skip empty memory types
                        if (typeStats.statistics.blockCount == 0) {
                            continue;
                        }
                        
                        // Get memory type properties
                        VkMemoryPropertyFlags propFlags = memProps->memoryTypes[j].propertyFlags;
                        
                        // Determine memory type characteristics
                        bool isDeviceLocal = (propFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;
                        bool isHostVisible = (propFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
                        bool isHostCoherent = (propFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
                        bool isHostCached = (propFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0;
                        
                        // Create a string describing the memory type
                        char typeDesc[128] = "";
                        if (isDeviceLocal) strcat(typeDesc, "DEVICE_LOCAL ");
                        if (isHostVisible) strcat(typeDesc, "HOST_VISIBLE ");
                        if (isHostCoherent) strcat(typeDesc, "HOST_COHERENT ");
                        if (isHostCached) strcat(typeDesc, "HOST_CACHED ");
                        
                        printf("    Type %u (%s):\n", j, typeDesc);
                        printf("      Allocations: %u\n", typeStats.statistics.allocationCount);
                        printf("      Blocks: %u\n", typeStats.statistics.blockCount);
                        printf("      Allocated: %0.1f MB\n", (double)typeStats.statistics.allocationBytes / (1024.0 * 1024.0));
                        printf("      Block memory: %0.1f MB\n", (double)typeStats.statistics.blockBytes / (1024.0 * 1024.0));
                        printf("      Unused range count: %u\n", typeStats.unusedRangeCount);
                        
                        if (typeStats.unusedRangeCount > 0) {
                            printf("      Unused range size: min=%0.1f KB, avg=%0.1f KB, max=%0.1f KB\n",
                                   (double)typeStats.unusedRangeSizeMin / 1024.0,
                                   typeStats.unusedRangeCount > 0 
                                       ? (double)(typeStats.statistics.blockBytes - typeStats.statistics.allocationBytes) / (double)typeStats.unusedRangeCount / 1024.0 
                                       : 0.0,
                                   (double)typeStats.unusedRangeSizeMax / 1024.0);
                        }
                    }
                }
            }
        }
        
        printf("\n");
    } catch (const std::exception& e) {
        printf("Error printing memory usage: %s\n", e.what());
    }
}

} // namespace ev 