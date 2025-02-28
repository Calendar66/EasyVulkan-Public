/**
 * @file ImageBuilder.hpp
 * @brief Builder class for creating Vulkan images in EasyVulkan framework
 * @details This file contains the ImageBuilder class which provides a fluent interface
 *          for creating and initializing Vulkan images with proper memory allocation
 *          and layout transitions.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace ev {

class VulkanDevice;
class VulkanContext;


/**
 * @class ImageBuilder
 * @brief Fluent builder interface for creating Vulkan images
 * @details ImageBuilder simplifies image creation by:
 *          - Providing a fluent interface for configuration
 *          - Managing VMA (Vulkan Memory Allocator) allocation
 *          - Handling image layout transitions
 *          - Supporting automatic data upload
 *          - Creating image views
 *          - Providing sensible defaults for common use cases
 *
 * Common usage patterns:
 * @code
 * // Create a 2D texture
 * auto texture = imageBuilder
 *      .setFormat(VK_FORMAT_R8G8B8A8_SRGB)
 *      .setExtent(width, height)
 *      .setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
 *      .build("myTexture");
 *
 * // Create a depth buffer

 * auto depthBuffer = imageBuilder
 *      .setFormat(VK_FORMAT_D32_SFLOAT)
 *      .setExtent(width, height)
 *      .setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
 *      .build("depthBuffer");
 *
 * // Create a cubemap
 * auto cubemap = imageBuilder
 *      .setFormat(VK_FORMAT_R8G8B8A8_SRGB)
 *      .setExtent(size, size)
 *      .setArrayLayers(6)
 *      .setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
 *      .build("cubemap");
 * @endcode
 */

class ImageBuilder {
public:
    /**
     * @brief Constructor for ImageBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    explicit ImageBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Default destructor
     */
    ~ImageBuilder() = default;

    /**
     * @brief Sets the type of image (1D, 2D, or 3D)
     * @param imageType Image type
     * @return Reference to this builder for method chaining
     */
    ImageBuilder& setImageType(VkImageType imageType);

    /**
     * @brief Sets the format of the image
     * @param format Image format
     * @return Reference to this builder for method chaining
     * 
     * Common formats:
     * - VK_FORMAT_R8G8B8A8_SRGB: 8-bit RGBA color with sRGB encoding
     * - VK_FORMAT_B8G8R8A8_SRGB: 8-bit BGRA color with sRGB encoding
     * - VK_FORMAT_D32_SFLOAT: 32-bit float depth
     * - VK_FORMAT_D24_UNORM_S8_UINT: 24-bit depth + 8-bit stencil
     */
    ImageBuilder& setFormat(VkFormat format);

    /**
     * @brief Sets the dimensions of the image
     * @param width Width in pixels
     * @param height Height in pixels
     * @param depth Depth in pixels (default: 1 for 2D images)
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if any dimension is 0
     */
    ImageBuilder& setExtent(uint32_t width, uint32_t height, uint32_t depth = 1);

    /**
     * @brief Sets the number of mipmap levels
     * @param mipLevels Number of mipmap levels
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if mipLevels is 0
     */
    ImageBuilder& setMipLevels(uint32_t mipLevels);

    /**
     * @brief Sets the number of array layers
     * @param arrayLayers Number of array layers (6 for cubemaps)
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if arrayLayers is 0
     */
    ImageBuilder& setArrayLayers(uint32_t arrayLayers);

    /**
     * @brief Sets the number of samples for multisampling
     * @param samples Number of samples per pixel
     * @return Reference to this builder for method chaining
     */
    ImageBuilder& setSamples(VkSampleCountFlagBits samples);

    /**
     * @brief Sets the tiling mode of the image
     * @param tiling Tiling mode (linear or optimal)
     * @return Reference to this builder for method chaining
     * 
     * @note VK_IMAGE_TILING_LINEAR is typically used for staging images
     *       VK_IMAGE_TILING_OPTIMAL is typically used for device-local images
     */
    ImageBuilder& setTiling(VkImageTiling tiling);

    /**
     * @brief Sets the usage flags for the image
     * @param usage Image usage flags
     * @return Reference to this builder for method chaining
     * 
     * Common usage flags:
     * - VK_IMAGE_USAGE_TRANSFER_SRC_BIT: Image can be used as transfer source
     * - VK_IMAGE_USAGE_TRANSFER_DST_BIT: Image can be used as transfer destination
     * - VK_IMAGE_USAGE_SAMPLED_BIT: Image can be sampled in shaders
     * - VK_IMAGE_USAGE_STORAGE_BIT: Image can be used as storage image
     * - VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT: Image can be used as color attachment
     * - VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT: Image can be used as depth/stencil attachment
     */
    ImageBuilder& setUsage(VkImageUsageFlags usage);

    /**
     * @brief Sets the memory property flags for the image
     * @param properties Memory property flags
     * @return Reference to this builder for method chaining
     * 
     * Common property combinations:
     * - VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: GPU-only memory (fastest)
     * - VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
     *   CPU-accessible memory for staging
     */
    ImageBuilder& setMemoryProperties(VkMemoryPropertyFlags properties);

    /**
     * @brief Sets the memory usage hint for VMA allocation
     * @param usage VMA memory usage hint
     * @return Reference to this builder for method chaining
     * 
     * Common usage values:
     * - VMA_MEMORY_USAGE_AUTO: Let VMA choose the best memory type
     * - VMA_MEMORY_USAGE_GPU_ONLY: GPU local memory for best performance
     * - VMA_MEMORY_USAGE_CPU_ONLY: CPU accessible memory for staging
     * - VMA_MEMORY_USAGE_CPU_TO_GPU: CPU visible memory for frequent updates
     * - VMA_MEMORY_USAGE_GPU_TO_CPU: GPU memory readable by CPU
     */
    ImageBuilder& setMemoryUsage(VmaMemoryUsage usage);


    /**
     * @brief Sets the allocation creation flags for VMA
     * @param flags VMA allocation creation flags
     * @return Reference to this builder for method chaining
     * 
     * Common flags:
     * - VMA_ALLOCATION_CREATE_MAPPED_BIT: Keep memory persistently mapped
     * - VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT: Optimize for sequential CPU writes
     * - VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT: Optimize for random CPU access
     * - VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT: Use dedicated memory allocation
     */
    ImageBuilder& setMemoryFlags(VmaAllocationCreateFlags flags);


    /**
     * @brief Sets the sharing mode for the image
     * @param sharingMode Sharing mode (exclusive or concurrent)
     * @return Reference to this builder for method chaining
     */
    ImageBuilder& setSharingMode(VkSharingMode sharingMode);

    /**
     * @brief Sets the queue family indices for concurrent sharing mode
     * @param queueFamilyIndices Vector of queue family indices
     * @return Reference to this builder for method chaining
     */
    ImageBuilder& setQueueFamilyIndices(
        const std::vector<uint32_t>& queueFamilyIndices);

    /**
     * @brief Sets the initial layout of the image
     * @param initialLayout Initial image layout
     * @return Reference to this builder for method chaining
     */
    ImageBuilder& setInitialLayout(VkImageLayout initialLayout);

    /**
     * @brief Builds the image with current configuration
     * @param name Optional name for resource tracking
     * @param outAllocation Optional pointer to receive VMA allocation handle
     * @return Created image info
     * @throws std::runtime_error if:
     *         - Image creation fails
     *         - Memory allocation fails
     *         - Required parameters are not set
     * 
     * Example:
     * @code
     * VmaAllocation allocation;
     * auto image = imageBuilder
     *     .setFormat(VK_FORMAT_R8G8B8A8_SRGB)
     *     .setExtent(width, height)
     *     .setUsage(VK_IMAGE_USAGE_SAMPLED_BIT)
     *     .build("myImage", &allocation);
     * @endcode
     */
    ImageInfo build(
        const std::string& name = "",
        VmaAllocation* outAllocation = nullptr);


    /**
     * @brief Builds the image and initializes it with data
     * @param data Pointer to the image data
     * @param dataSize Size of the data in bytes
     * @param name Optional name for resource tracking
     * @param outAllocation Optional pointer to receive VMA allocation handle
     * @return Created and initialized image info
     * @throws std::runtime_error if:
     *         - Image creation fails
     *         - Memory allocation fails
     *         - Data upload fails
     *         - Data pointer is null
     *         - Data size doesn't match image size
     * 
     * Example:
     * @code
     * auto image = imageBuilder
     *     .setFormat(VK_FORMAT_R8G8B8A8_SRGB)
     *     .setExtent(width, height)
     *     .setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
     *     .buildAndInitialize(pixels, width * height * 4, "texture");
     * @endcode
     */
    ImageInfo buildAndInitialize(
        const void* data,
        VkDeviceSize dataSize,
        const std::string& name = "",
        VmaAllocation* outAllocation = nullptr);


    /**
     * @brief Creates an image view for an image
     * @param image Image handle
     * @param viewType Type of image view
     * @param aspectMask Image aspect mask
     * @param name Optional name for resource tracking
     * @return Created image view handle
     * @throws std::runtime_error if:
     *         - Image view creation fails
     *         - Image handle is invalid
     * 
     * Example:
     * @code
     * // Create a 2D texture view
     * auto imageView = imageBuilder->createImageView(
     *     image,
     *     VK_IMAGE_VIEW_TYPE_2D,
     *     VK_IMAGE_ASPECT_COLOR_BIT,
     *     "textureView"
     * );
     * 
     * // Create a cubemap view
     * auto cubemapView = imageBuilder->createImageView(
     *     image,
     *     VK_IMAGE_VIEW_TYPE_CUBE,
     *     VK_IMAGE_ASPECT_COLOR_BIT,
     *     "cubemapView"
     * );
     * @endcode
     */
    VkImageView createImageView(
        VkImage image,
        VkImageViewType viewType,
        VkImageAspectFlags aspectMask,
        const std::string& name = "");

private:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance
    VulkanContext* m_context;                ///< Pointer to VulkanContext instance

    // Image creation parameters
    VkImageType m_imageType{VK_IMAGE_TYPE_2D};  ///< Type of image (1D/2D/3D)
    VkFormat m_format{VK_FORMAT_UNDEFINED};      ///< Image format
    VkExtent3D m_extent{1, 1, 1};               ///< Image dimensions
    uint32_t m_mipLevels{1};                    ///< Number of mipmap levels
    uint32_t m_arrayLayers{1};                  ///< Number of array layers
    VkSampleCountFlagBits m_samples{VK_SAMPLE_COUNT_1_BIT}; ///< Sample count
    VkImageTiling m_tiling{VK_IMAGE_TILING_OPTIMAL}; ///< Image tiling mode
    VkImageUsageFlags m_usage{0};                ///< Image usage flags
    VmaMemoryUsage m_memoryUsage{VMA_MEMORY_USAGE_AUTO}; ///< Memory usage hint
    VmaAllocationCreateFlags m_memoryFlags{0}; ///< Allocation creation flags
    VkMemoryPropertyFlags m_memoryProperties{VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}; ///< Memory properties
    VkSharingMode m_sharingMode{VK_SHARING_MODE_EXCLUSIVE}; ///< Sharing mode
    std::vector<uint32_t> m_queueFamilyIndices; ///< Queue families for concurrent sharing

    VkImageLayout m_initialLayout{VK_IMAGE_LAYOUT_UNDEFINED}; ///< Initial image layout

    /**
     * @brief Validates builder parameters before image creation
     * @throws std::runtime_error if parameters are invalid
     */
    void validateParameters() const;

    /**
     * @brief Creates the image using VMA
     * @param outAllocation Pointer to receive VMA allocation handle
     * @return Created image
     * @throws std::runtime_error if image creation fails
     */
    VkImage createImage(VmaAllocation* outAllocation) const;

    /**
     * @brief Uploads data to an image
     * @param imageInfo ImageInfo to upload to
     * @param data Pointer to image data
     * @param dataSize Size of data in bytes
     * @throws std::runtime_error if data upload fails
     */

    void uploadData(
        ImageInfo imageInfo,
        const void* data,
        VkDeviceSize dataSize) const;

    /**
     * @brief Transitions an image's layout
     * @param image Image to transition
     * @param oldLayout Current layout
     * @param newLayout Desired layout
     * @throws std::runtime_error if layout transition fails
     */
    void transitionImageLayout(
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout) const;
};

} // namespace ev 