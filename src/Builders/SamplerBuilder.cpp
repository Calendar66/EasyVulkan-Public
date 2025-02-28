#include "EasyVulkan/Builders/SamplerBuilder.hpp"

#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/VulkanContext.hpp"

#include "EasyVulkan/Core/ResourceManager.hpp"
#include <stdexcept>

namespace ev {

SamplerBuilder::SamplerBuilder(VulkanDevice* device,VulkanContext* context)
    : m_device(device),m_context(context) {
}

SamplerBuilder& SamplerBuilder::setMagFilter(VkFilter filter) {
    m_magFilter = filter;
    return *this;
}

SamplerBuilder& SamplerBuilder::setMinFilter(VkFilter filter) {
    m_minFilter = filter;
    return *this;
}

SamplerBuilder& SamplerBuilder::setMipmapMode(VkSamplerMipmapMode mode) {
    m_mipmapMode = mode;
    return *this;
}

SamplerBuilder& SamplerBuilder::setAddressModeU(VkSamplerAddressMode mode) {
    m_addressModeU = mode;
    return *this;
}

SamplerBuilder& SamplerBuilder::setAddressModeV(VkSamplerAddressMode mode) {
    m_addressModeV = mode;
    return *this;
}

SamplerBuilder& SamplerBuilder::setAddressModeW(VkSamplerAddressMode mode) {
    m_addressModeW = mode;
    return *this;
}

SamplerBuilder& SamplerBuilder::setAnisotropy(float maxAnisotropy) {
    m_anisotropyEnable = maxAnisotropy > 1.0f ? VK_TRUE : VK_FALSE;
    m_maxAnisotropy = maxAnisotropy;
    return *this;
}

SamplerBuilder& SamplerBuilder::setBorderColor(VkBorderColor borderColor) {
    m_borderColor = borderColor;
    return *this;
}

SamplerBuilder& SamplerBuilder::setCompareOp(VkCompareOp compareOp) {
    m_compareEnable = compareOp != VK_COMPARE_OP_NEVER ? VK_TRUE : VK_FALSE;
    m_compareOp = compareOp;
    return *this;
}

SamplerBuilder& SamplerBuilder::setLodBias(float bias) {
    m_mipLodBias = bias;
    return *this;
}

SamplerBuilder& SamplerBuilder::setMinLod(float minLod) {
    m_minLod = minLod;
    return *this;
}

SamplerBuilder& SamplerBuilder::setMaxLod(float maxLod) {
    m_maxLod = maxLod;
    return *this;
}

SamplerBuilder& SamplerBuilder::setUnnormalizedCoordinates(VkBool32 unnormalized) {
    m_unnormalizedCoordinates = unnormalized;
    return *this;
}

void SamplerBuilder::validateParameters() const {
    // Check if anisotropy is supported and within device limits
    if (m_anisotropyEnable == VK_TRUE) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(m_device->getPhysicalDevice(), &properties);
        
        if (m_maxAnisotropy > properties.limits.maxSamplerAnisotropy) {
            throw std::runtime_error("Requested anisotropy level exceeds device limits");
        }
    }

    // Check if unnormalized coordinates are compatible with other settings
    if (m_unnormalizedCoordinates == VK_TRUE) {
        if (m_anisotropyEnable == VK_TRUE) {
            throw std::runtime_error("Anisotropic filtering cannot be used with unnormalized coordinates");
        }
        if (m_compareEnable == VK_TRUE) {
            throw std::runtime_error("Compare operations cannot be used with unnormalized coordinates");
        }
        if (m_mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
            throw std::runtime_error("Only nearest mipmap mode can be used with unnormalized coordinates");
        }
    }
}

VkSampler SamplerBuilder::build(const std::string& name) {
    validateParameters();

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = m_magFilter;
    samplerInfo.minFilter = m_minFilter;
    samplerInfo.mipmapMode = m_mipmapMode;
    samplerInfo.addressModeU = m_addressModeU;
    samplerInfo.addressModeV = m_addressModeV;
    samplerInfo.addressModeW = m_addressModeW;
    samplerInfo.mipLodBias = m_mipLodBias;
    samplerInfo.anisotropyEnable = m_anisotropyEnable;
    samplerInfo.maxAnisotropy = m_maxAnisotropy;
    samplerInfo.compareEnable = m_compareEnable;
    samplerInfo.compareOp = m_compareOp;
    samplerInfo.minLod = m_minLod;
    samplerInfo.maxLod = m_maxLod;
    samplerInfo.borderColor = m_borderColor;
    samplerInfo.unnormalizedCoordinates = m_unnormalizedCoordinates;

    VkSampler sampler;
    if (vkCreateSampler(m_device->getLogicalDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create sampler!");
    }

    // Register the sampler for resource tracking if a name is provided
    if (!name.empty()) {
        m_context->getResourceManager()->registerResource(
            name, reinterpret_cast<uint64_t>(sampler), VK_OBJECT_TYPE_SAMPLER);
    }

    return sampler;
}

} // namespace ev 