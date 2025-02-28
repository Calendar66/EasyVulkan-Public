/**
 * @file SamplerBuilder.hpp
 * @brief Builder class for creating samplers in EasyVulkan framework
 * @details This file contains the SamplerBuilder class which provides a fluent interface
 *          for creating Vulkan samplers with configurable filtering, addressing, and LOD parameters.
 */

#pragma once

#include "../Common.hpp"

namespace ev {

class VulkanDevice;
class VulkanContext;

/**
 * @class SamplerBuilder
 * @brief Fluent builder interface for creating Vulkan samplers
 * @details SamplerBuilder simplifies sampler creation by:
 *          - Providing a fluent interface for configuration
 *          - Managing sampler parameters
 *          - Providing sensible defaults for common use cases
 *          - Supporting all sampler features (filtering, addressing, anisotropy, etc.)
 *          - Validating sampler configuration
 *
 * Common usage patterns:
 * @code
 * // Create a basic linear sampler
 * auto sampler = samplerBuilder
 *     .setMagFilter(VK_FILTER_LINEAR)
 *     .setMinFilter(VK_FILTER_LINEAR)
 *     .build("basicSampler");
 *
 * // Create a sampler with anisotropic filtering
 * auto anisotropicSampler = samplerBuilder
 *     .setMagFilter(VK_FILTER_LINEAR)
 *     .setMinFilter(VK_FILTER_LINEAR)
 *     .setAnisotropy(16.0f)
 *     .build("anisotropicSampler");
 *
 * // Create a sampler for shadow mapping
 * auto shadowSampler = samplerBuilder
 *     .setMagFilter(VK_FILTER_LINEAR)
 *     .setMinFilter(VK_FILTER_LINEAR)
 *     .setAddressModeU(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
 *     .setAddressModeV(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
 *     .setBorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
 *     .setCompareOp(VK_COMPARE_OP_LESS)
 *     .build("shadowSampler");
 * @endcode
 */
class SamplerBuilder {
public:
    /**
     * @brief Constructor for SamplerBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    explicit SamplerBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Virtual destructor for proper cleanup
     */
    ~SamplerBuilder() = default;

    /**
     * @brief Sets the magnification filter mode
     * @param filter Filter mode for texture magnification
     * @return Reference to this builder for method chaining
     *
     * Common filter modes:
     * - VK_FILTER_NEAREST: Nearest neighbor filtering
     * - VK_FILTER_LINEAR: Linear interpolation filtering
     */
    SamplerBuilder& setMagFilter(VkFilter filter);

    /**
     * @brief Sets the minification filter mode
     * @param filter Filter mode for texture minification
     * @return Reference to this builder for method chaining
     *
     * Common filter modes:
     * - VK_FILTER_NEAREST: Nearest neighbor filtering
     * - VK_FILTER_LINEAR: Linear interpolation filtering
     */
    SamplerBuilder& setMinFilter(VkFilter filter);

    /**
     * @brief Sets the mipmap mode
     * @param mode Mipmap interpolation mode
     * @return Reference to this builder for method chaining
     *
     * Mipmap modes:
     * - VK_SAMPLER_MIPMAP_MODE_NEAREST: Use nearest mipmap level
     * - VK_SAMPLER_MIPMAP_MODE_LINEAR: Interpolate between mipmap levels
     */
    SamplerBuilder& setMipmapMode(VkSamplerMipmapMode mode);

    /**
     * @brief Sets the addressing mode for U coordinates
     * @param mode Addressing mode for U coordinates
     * @return Reference to this builder for method chaining
     *
     * Common address modes:
     * - VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat texture
     * - VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Mirror and repeat
     * - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Clamp to edge pixels
     * - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Use border color
     */
    SamplerBuilder& setAddressModeU(VkSamplerAddressMode mode);

    /**
     * @brief Sets the addressing mode for V coordinates
     * @param mode Addressing mode for V coordinates
     * @return Reference to this builder for method chaining
     */
    SamplerBuilder& setAddressModeV(VkSamplerAddressMode mode);

    /**
     * @brief Sets the addressing mode for W coordinates
     * @param mode Addressing mode for W coordinates
     * @return Reference to this builder for method chaining
     */
    SamplerBuilder& setAddressModeW(VkSamplerAddressMode mode);

    /**
     * @brief Enables and sets anisotropic filtering
     * @param maxAnisotropy Maximum anisotropy value (typically 1.0 to 16.0)
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if maxAnisotropy is less than 1.0
     *
     * @note Setting maxAnisotropy > 1.0 automatically enables anisotropic filtering
     */
    SamplerBuilder& setAnisotropy(float maxAnisotropy);

    /**
     * @brief Sets the border color for clamp to border addressing mode
     * @param borderColor Predefined border color
     * @return Reference to this builder for method chaining
     *
     * Common border colors:
     * - VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK
     * - VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK
     * - VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
     */
    SamplerBuilder& setBorderColor(VkBorderColor borderColor);

    /**
     * @brief Sets the comparison operation for shadow mapping
     * @param compareOp Comparison operation
     * @return Reference to this builder for method chaining
     *
     * @note Setting a compare op automatically enables comparison mode
     */
    SamplerBuilder& setCompareOp(VkCompareOp compareOp);

    /**
     * @brief Sets the LOD bias value
     * @param bias LOD bias value
     * @return Reference to this builder for method chaining
     */
    SamplerBuilder& setLodBias(float bias);

    /**
     * @brief Sets the minimum LOD value
     * @param minLod Minimum LOD value
     * @return Reference to this builder for method chaining
     */
    SamplerBuilder& setMinLod(float minLod);

    /**
     * @brief Sets the maximum LOD value
     * @param maxLod Maximum LOD value
     * @return Reference to this builder for method chaining
     */
    SamplerBuilder& setMaxLod(float maxLod);

    /**
     * @brief Sets whether to use unnormalized coordinates
     * @param unnormalized Whether to use unnormalized coordinates
     * @return Reference to this builder for method chaining
     *
     * @note Unnormalized coordinates use absolute texel coordinates
     *       instead of [0,1] normalized coordinates
     */
    SamplerBuilder& setUnnormalizedCoordinates(VkBool32 unnormalized);

    /**
     * @brief Builds the sampler with current configuration
     * @param name Optional name for resource tracking
     * @return Created sampler handle
     * @throws std::runtime_error if:
     *         - Device limits are exceeded
     *         - Parameters are invalid
     *         - Sampler creation fails
     */
    VkSampler build(const std::string& name = "");

private:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance
    VulkanContext* m_context;                ///< Pointer to VulkanContext instance

    // Sampler parameters
    VkFilter m_magFilter{VK_FILTER_LINEAR};  ///< Magnification filter mode
    VkFilter m_minFilter{VK_FILTER_LINEAR};  ///< Minification filter mode
    VkSamplerMipmapMode m_mipmapMode{VK_SAMPLER_MIPMAP_MODE_LINEAR}; ///< Mipmap interpolation mode
    VkSamplerAddressMode m_addressModeU{VK_SAMPLER_ADDRESS_MODE_REPEAT}; ///< U coordinate addressing
    VkSamplerAddressMode m_addressModeV{VK_SAMPLER_ADDRESS_MODE_REPEAT}; ///< V coordinate addressing
    VkSamplerAddressMode m_addressModeW{VK_SAMPLER_ADDRESS_MODE_REPEAT}; ///< W coordinate addressing
    float m_mipLodBias{0.0f};               ///< LOD bias value
    VkBool32 m_anisotropyEnable{VK_FALSE};  ///< Whether anisotropic filtering is enabled
    float m_maxAnisotropy{1.0f};            ///< Maximum anisotropy value
    VkBool32 m_compareEnable{VK_FALSE};     ///< Whether comparison mode is enabled
    VkCompareOp m_compareOp{VK_COMPARE_OP_NEVER}; ///< Comparison operation
    float m_minLod{0.0f};                   ///< Minimum LOD value
    float m_maxLod{VK_LOD_CLAMP_NONE};      ///< Maximum LOD value
    VkBorderColor m_borderColor{VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK}; ///< Border color
    VkBool32 m_unnormalizedCoordinates{VK_FALSE}; ///< Whether to use unnormalized coordinates

    /**
     * @brief Validates the current builder state
     * @throws std::runtime_error if:
     *         - Parameters exceed device limits
     *         - Parameter combinations are invalid
     */
    void validateParameters() const;
};

} // namespace ev 