/**
 * @file ShaderModuleBuilder.hpp
 * @brief Builder class for creating shader modules in EasyVulkan framework
 * @details This file contains the ShaderModuleBuilder class which provides a fluent
 *          interface for creating Vulkan shader modules from SPIR-V code or files.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace ev {

class VulkanDevice;
class VulkanContext;

/**
 * @class ShaderModuleBuilder
 * @brief Fluent builder interface for creating Vulkan shader modules
 * @details ShaderModuleBuilder simplifies shader module creation by:
 *          - Providing a fluent interface for configuration
 *          - Supporting direct SPIR-V code loading
 *          - Supporting SPIR-V file loading
 *          - Handling resource tracking
 *          - Validating shader code
 *
 * Common usage patterns:
 * @code
 * // Load shader from SPIR-V file
 * auto vertShader = shaderBuilder
 *     ->loadFromFile("shaders/vert.spv")
 *     ->build("vertexShader");
 *
 * // Create shader from SPIR-V code
 * std::vector<uint32_t> spirvCode = ...;
 * auto fragShader = shaderBuilder
 *     ->setCode(spirvCode)
 *     ->build("fragmentShader");
 *
 * // Create shader from raw SPIR-V data
 * const void* rawData = ...;
 * size_t dataSize = ...;
 * auto compShader = shaderBuilder
 *     ->setCode(rawData, dataSize)
 *     ->build("computeShader");
 * @endcode
 */
class ShaderModuleBuilder {
public:
    /**
     * @brief Constructor for ShaderModuleBuilder
     * @param device Pointer to VulkanDevice instance
     * @param context Pointer to VulkanContext instance
     * @throws std::runtime_error if either pointer is null
     */
    explicit ShaderModuleBuilder(VulkanDevice* device, VulkanContext* context);

    /**
     * @brief Default destructor
     */
    ~ShaderModuleBuilder() = default;

    /**
     * @brief Sets the SPIR-V code from a vector
     * @param code Vector containing SPIR-V bytecode
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Code vector is empty
     *         - Code size is not a multiple of 4
     *
     * Example:
     * @code
     * std::vector<uint32_t> spirvCode = loadShaderCode("shader.spv");
     * auto shaderModule = builder
     *     ->setCode(spirvCode)
     *     ->build("myShader");
     * @endcode
     */
    ShaderModuleBuilder& setCode(const std::vector<uint32_t>& code);

    /**
     * @brief Sets the SPIR-V code from raw data
     * @param pCode Pointer to SPIR-V bytecode
     * @param codeSize Size of the code in bytes
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - Code pointer is null
     *         - Code size is 0
     *         - Code size is not a multiple of 4
     *
     * Example:
     * @code
     * const void* rawSpirv = getRawShaderData();
     * size_t dataSize = getShaderDataSize();
     * auto shaderModule = builder
     *     ->setCode(rawSpirv, dataSize)
     *     ->build("myShader");
     * @endcode
     */
    ShaderModuleBuilder& setCode(const void* pCode, size_t codeSize);

    /**
     * @brief Loads SPIR-V code from a file
     * @param filename Path to the SPIR-V shader file
     * @return Reference to this builder for method chaining
     * @throws std::runtime_error if:
     *         - File cannot be opened
     *         - File is empty
     *         - File size is not a multiple of 4
     *         - File read fails
     *
     * Example:
     * @code
     * // Load vertex shader
     * auto vertModule = builder
     *     ->loadFromFile("shaders/vertex.spv")
     *     ->build("vertexShader");
     *
     * // Load fragment shader
     * auto fragModule = builder
     *     ->loadFromFile("shaders/fragment.spv")
     *     ->build("fragmentShader");
     * @endcode
     */
    ShaderModuleBuilder& loadFromFile(const std::string& filename);

    /**
     * @brief Builds the shader module with current configuration
     * @param name Optional name for resource tracking and debugging
     * @return Created shader module handle
     * @throws std::runtime_error if:
     *         - No shader code has been set
     *         - Shader module creation fails
     *         - Code validation fails
     *
     * Example:
     * @code
     * // Create a named shader module
     * auto shaderModule = builder
     *     ->loadFromFile("shader.spv")
     *     ->build("mainShader");
     *
     * // Create an unnamed shader module
     * auto tempModule = builder
     *     ->setCode(spirvCode)
     *     ->build();
     * @endcode
     */
    VkShaderModule build(const std::string& name = "");

private:
    VulkanDevice* m_device;                  ///< Pointer to VulkanDevice instance
    VulkanContext* m_context;                ///< Pointer to VulkanContext instance
    std::vector<uint32_t> m_code;            ///< SPIR-V bytecode

    /**
     * @brief Validates builder parameters before shader module creation
     * @throws std::runtime_error if parameters are invalid
     */
    void validateParameters() const;

    /**
     * @brief Loads SPIR-V code from a file
     * @param filename Path to the SPIR-V file
     * @return Vector containing the SPIR-V bytecode
     * @throws std::runtime_error if file loading fails
     */
    std::vector<uint32_t> loadSPIRVFromFile(const std::string& filename) const;
};

} // namespace ev 