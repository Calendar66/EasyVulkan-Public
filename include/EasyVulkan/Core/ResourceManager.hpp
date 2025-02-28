/**
 * @file ResourceManager.hpp
 * @brief Resource management class for EasyVulkan framework
 * @details This file contains the ResourceManager class which provides a centralized
 *          system for creating, tracking, and managing Vulkan resources through
 *          builder patterns.
 */

#pragma once

#include "../Common.hpp"
#include "../DataStructures.hpp"

#include <memory>
#include <string>
#include <unordered_map>


namespace ev {

// Forward declarations
class VulkanDevice;
class VulkanContext;
class BufferBuilder;
class ImageBuilder;
class GraphicsPipelineBuilder;
class ComputePipelineBuilder;
class DescriptorSetBuilder;
class RenderPassBuilder;
class FramebufferBuilder;
class CommandBufferBuilder;
class SamplerBuilder;
class ShaderModuleBuilder;

/**
 * @class ResourceManager
 * @brief Central manager for Vulkan resource creation and lifecycle management
 * @details ResourceManager provides:
 *          - Builder interfaces for all major Vulkan resources
 *          - Automatic resource tracking and cleanup
 *          - Name-based resource lookup
 *          - RAII-style resource management
 *
 * The manager uses builder patterns to simplify resource creation:
 * @code
 * // Create a buffer with builder pattern
 * auto buffer = resourceManager->createBuffer()
 *     .setSize(size)
 *     .setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
 *     .setMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
 *     .build("myVertexBuffer");
 *
 * // Create an image with builder pattern
 * auto image = resourceManager->createImage()
 *     .setDimensions(width, height)
 *     .setFormat(VK_FORMAT_R8G8B8A8_SRGB)
 *     .setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
 *     .build("myTexture");
 * @endcode
 *
 * @note Inheritance:
 *       - Override resource creation methods for custom allocation strategies
 *       - Override cleanup behavior for specialized resource tracking
 *       - Override registerResource for custom resource naming/debugging
 */
class ResourceManager {
public:
    VulkanDevice* m_device;  ///< Pointer to VulkanDevice instance
    VulkanContext* m_context;  ///< Pointer to VulkanContext instance

    // Resource tracking maps
    std::unordered_map<std::string, VkBuffer> m_buffers;           ///< Buffer handles
    std::unordered_map<std::string, ImageInfo> m_images;            ///< Image handles
    std::unordered_map<std::string, VkDescriptorSetLayout> m_descriptorSetLayouts; ///< Descriptor set layout handles
    std::unordered_map<std::string, VkDescriptorSet> m_descriptorSets; ///< Descriptor set handles
    std::unordered_map<std::string, VkRenderPass> m_renderPasses; ///< Render pass handles
    std::unordered_map<std::string, VkFramebuffer> m_framebuffers; ///< Framebuffer handles
    std::unordered_map<std::string, VkSampler> m_samplers;        ///< Sampler handles
    std::unordered_map<std::string, VkShaderModule> m_shaderModules; ///< Shader module handles
    std::unordered_map<std::string, CommandBufferInfo> m_commandBuffers; ///< Command buffer info
    std::unordered_map<std::string, PipelineInfo> m_pipelines; ///< Pipeline info

    

    /**
     * @brief Constructor for ResourceManager
     * @param device Pointer to VulkanDevice instance
     * @throws std::runtime_error if device is nullptr
     */
    explicit ResourceManager(VulkanDevice* device, VulkanContext* context);
    
    /**
     * @brief Virtual destructor for proper cleanup
     * @details Automatically cleans up all tracked resources in the correct order
     */
    virtual ~ResourceManager();

    /**
     * @brief Creates a buffer builder for vertex, index, uniform, or storage buffers
     * @return Reference to BufferBuilder
     * 
     * Example usage:
     * @code
     * // Create a vertex buffer
     * auto vertexBuffer = resourceManager->createBuffer()
     *     .setSize(sizeof(vertices))
     *     .setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
     *     .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
     *                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
     *     .build("vertices");
     * @endcode
     */
    virtual BufferBuilder& createBuffer();

    /**
     * @brief Creates an image builder for textures, attachments, etc.
     * @return Reference to ImageBuilder
     * 
     * Example usage:
     * @code
     * // Create a texture image
     * auto textureImage = resourceManager->createImage()
     *     .setDimensions(texWidth, texHeight)
     *     .setFormat(VK_FORMAT_R8G8B8A8_SRGB)
     *     .setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
     *                VK_IMAGE_USAGE_SAMPLED_BIT)
     *     .setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
     *     .build("texture");
     * @endcode
     */
    virtual ImageBuilder& createImage();

    /**
     * @brief Creates a graphics pipeline builder
     * @return Reference to GraphicsPipelineBuilder
     * 
     * Example usage:
     * @code
     * auto pipeline = resourceManager->createGraphicsPipeline()
     *     .addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertModule)
     *     .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragModule)
     *     .setVertexInputState(bindings, attributes)
     *     .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
     *     .setViewport(viewport)
     *     .setScissor(scissor)
     *     .setRasterizationState()
     *     .setMultisampleState()
     *     .setColorBlendState(attachments)
     *     .setLayout(pipelineLayout)
     *     .setRenderPass(renderPass)
     *     .build("mainPipeline");
     * @endcode
     */
    virtual GraphicsPipelineBuilder& createGraphicsPipeline();

    /**
     * @brief Creates a compute pipeline builder
     * @return Reference to ComputePipelineBuilder
     * 
     * Example usage:
     * @code
     * auto pipeline = resourceManager->createComputePipeline()
     *     .setShaderStage(computeModule)
     *     .setLayout(pipelineLayout)
     *     .build("computePipeline");
     * @endcode
     */
    virtual ComputePipelineBuilder& createComputePipeline();

    /**
     * @brief Creates a descriptor set builder
     * @return Reference to DescriptorSetBuilder
     * 
     * Example usage:
     * @code
     * auto descriptorSet = resourceManager->createDescriptorSet()
     *     .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
     *                 VK_SHADER_STAGE_VERTEX_BIT)
     *     .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
     *                 VK_SHADER_STAGE_FRAGMENT_BIT)
     *     .addBufferDescriptor(0, uniformBuffer, 0, sizeof(UniformData))
     *     .addImageDescriptor(1, imageView, sampler,
     *                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
     *     .build("mainDescriptorSet");
     * @endcode
     */
    virtual DescriptorSetBuilder& createDescriptorSet();

    /**
     * @brief Creates a render pass builder
     * @return Reference to RenderPassBuilder
     * 
     * Example usage:
     * @code
     * auto renderPass = resourceManager->createRenderPass()
     *     .addColorAttachment(swapchainFormat,
     *                         VK_IMAGE_LAYOUT_UNDEFINED,
     *                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
     *     .addDepthAttachment(depthFormat,
     *                         VK_IMAGE_LAYOUT_UNDEFINED,
     *                         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
     *     .build("mainRenderPass");
     * @endcode
     */
    virtual RenderPassBuilder& createRenderPass();

    /**
     * @brief Creates a framebuffer builder
     * @return Reference to FramebufferBuilder
     * 
     * Example usage:
     * @code
     * auto framebuffer = resourceManager->createFramebuffer()
     *     .setRenderPass(renderPass)
     *     .addAttachment(colorImageView)
     *     .addAttachment(depthImageView)
     *     .setDimensions(width, height)
     *     .build("mainFramebuffer");
     * @endcode
     */
    virtual FramebufferBuilder& createFramebuffer();

    /**
     * @brief Creates a command buffer builder
     * @return Reference to CommandBufferBuilder
     * 
     * Example usage:
     * @code
     * auto cmdBuffer = resourceManager->createCommandBuffer()
     *     .setPool(commandPool)
     *     .setLevel(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
     *     .build("mainCommandBuffer");
     * @endcode
     */
    virtual CommandBufferBuilder& createCommandBuffer();

    /**
     * @brief Creates a sampler builder
     * @return Reference to SamplerBuilder
     * 
     * Example usage:
     * @code
     * auto sampler = resourceManager->createSampler()
     *     .setMinFilter(VK_FILTER_LINEAR)
     *     .setMagFilter(VK_FILTER_LINEAR)
     *     .setAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
     *     .setAnisotropy(16.0f)
     *     .build("textureSampler");
     * @endcode
     */
    virtual SamplerBuilder& createSampler();

    /**
     * @brief Creates a shader module builder
     * @return Reference to ShaderModuleBuilder
     * 
     * Example usage:
     * @code
     * auto vertShader = resourceManager->createShaderModule()
     *     .loadFromFile("shaders/vert.spv")
     *     .build("vertexShader");
     * @endcode
     */
    virtual ShaderModuleBuilder& createShaderModule();

    /**
     * @brief Registers a resource for tracking and debugging
     * @param name Resource name/identifier
     * @param handle Raw Vulkan handle
     * @param type Vulkan object type
     * @throws std::runtime_error if resource registration fails
     */
    virtual void registerResource(const std::string& name, uint64_t handle,
                                VkObjectType type);

    /**
     * @brief Registers a resource for tracking and debugging with two handles
     * @param name Resource name/identifier
     * @param primaryHandle Raw Vulkan handle
     * @param secondaryHandle Raw Vulkan handle
     * @param type Vulkan object type
     * @throws std::runtime_error if resource registration fails
     */
    virtual void registerResource2(const std::string& name, uint64_t primaryHandle,
                                uint64_t secondaryHandle, VkObjectType type);

private:
    /**
     * @brief Cleans up all tracked resources
     * @details Resources are destroyed in the correct order to prevent validation errors
     */
    void cleanup();

    /**
     * @brief Destroys a single resource
     * @param handle Raw Vulkan handle
     * @param type Vulkan object type
     */
    void destroyResource(uint64_t handle, VkObjectType type);
};

} // namespace ev