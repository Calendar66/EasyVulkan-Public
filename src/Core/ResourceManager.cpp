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
#include <stdexcept>

namespace ev {

ResourceManager::ResourceManager(VulkanDevice* device, VulkanContext* context)
    : m_device(device)
    , m_context(context) {
}

ResourceManager::~ResourceManager() {
    cleanup();
}

BufferBuilder& ResourceManager::createBuffer() {
    return *new BufferBuilder(m_device,m_context);
}

ImageBuilder& ResourceManager::createImage() {
    return *new ImageBuilder(m_device,m_context);
}

GraphicsPipelineBuilder& ResourceManager::createGraphicsPipeline() {
    return *new GraphicsPipelineBuilder(m_device,m_context);
}

ComputePipelineBuilder& ResourceManager::createComputePipeline() {
    return *new ComputePipelineBuilder(m_device,m_context);
}

DescriptorSetBuilder& ResourceManager::createDescriptorSet() {
    return *new DescriptorSetBuilder(m_device,m_context);
}

RenderPassBuilder& ResourceManager::createRenderPass() {
    return *new RenderPassBuilder(m_device,m_context);
}

FramebufferBuilder& ResourceManager::createFramebuffer() {
    return *new FramebufferBuilder(m_device,m_context);
}

CommandBufferBuilder& ResourceManager::createCommandBuffer() {
    return *new CommandBufferBuilder(m_device,m_context);
}

SamplerBuilder& ResourceManager::createSampler() {
    return *new SamplerBuilder(m_device,m_context);
}

ShaderModuleBuilder& ResourceManager::createShaderModule() {
    return *new ShaderModuleBuilder(m_device,m_context);
}

void ResourceManager::registerResource(const std::string& name, uint64_t handle, VkObjectType type) {
    if (name.empty()) {
        return;
    }

    switch (type) {
        case VK_OBJECT_TYPE_BUFFER:
            m_buffers[name] = reinterpret_cast<VkBuffer>(handle);
            break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:
            m_descriptorSets[name] = reinterpret_cast<VkDescriptorSet>(handle);
            break;
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
        case VK_OBJECT_TYPE_PIPELINE:
        case VK_OBJECT_TYPE_IMAGE:
        case VK_OBJECT_TYPE_COMMAND_BUFFER: {
            LogError("This kind of resource tracking should be done with registerResource2()");
            break;
        }
        default:
            LogError("Unsupported resource type for tracking");
            throw std::runtime_error("Unsupported resource type for tracking");
    }
}

void ResourceManager::registerResource2(const std::string& name, uint64_t primaryHandle,
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
        case VK_OBJECT_TYPE_IMAGE:
            ImageInfo imageInfo;
            imageInfo.image = reinterpret_cast<VkImage>(primaryHandle);
            imageInfo.imageView = reinterpret_cast<VkImageView>(secondaryHandle);
            m_images[name] = imageInfo;
            break;
        case VK_OBJECT_TYPE_BUFFER:
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:
        case VK_OBJECT_TYPE_RENDER_PASS:
        case VK_OBJECT_TYPE_FRAMEBUFFER:
        case VK_OBJECT_TYPE_SAMPLER:
        case VK_OBJECT_TYPE_SHADER_MODULE:
            LogError("This kind of resource tracking should be done with registerResource()");
            break;
        default:
            throw std::runtime_error("Unsupported resource type for tracking");
    }
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
        vkDestroyImage(device, pair.second.image, nullptr);
        vkDestroyImageView(device, pair.second.imageView, nullptr);
    }
    m_images.clear();

    for (const auto& pair : m_buffers) {
        vkDestroyBuffer(device, pair.second, nullptr);
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

    // Note: Descriptor sets are freed with their pool, not individually
    // TODO: Free descriptor sets
    m_descriptorSets.clear();
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


} // namespace ev 