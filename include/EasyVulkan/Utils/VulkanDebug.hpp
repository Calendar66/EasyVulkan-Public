/**
 * @file VulkanDebug.hpp
 * @brief Debug utilities for Vulkan in EasyVulkan framework
 * @details This file contains utilities for Vulkan debugging, validation layers,
 *          object naming, and debug markers/labels for profiling and debugging.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace ev {

/**
 * @namespace VulkanDebug
 * @brief Namespace containing Vulkan debugging utilities
 * @details Provides functionality for:
 *          - Validation layer setup and management
 *          - Debug messenger creation and configuration
 *          - Object naming for debugging
 *          - Debug markers and regions for GPU profiling
 *          - Validation layer checking
 *
 * Common usage patterns:
 * @code
 * // Setup validation layers
 * VkDebugUtilsMessengerCreateInfoEXT createInfo{};
 * VulkanDebug::populateDebugMessengerCreateInfo(createInfo);
 * 
 * // Create debug messenger
 * VkDebugUtilsMessengerEXT debugMessenger;
 * VulkanDebug::createDebugUtilsMessengerEXT(
 *     instance, &createInfo, nullptr, &debugMessenger);
 *
 * // Name objects for debugging
 * VulkanDebug::setDebugObjectName(
 *     device,
 *     VK_OBJECT_TYPE_BUFFER,
 *     (uint64_t)buffer,
 *     "MyVertexBuffer"
 * );
 *
 * // Use debug markers in command buffers
 * float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};  // Red
 * VulkanDebug::beginDebugLabel(device, cmdBuffer, "Draw Scene", color);
 * // ... render scene ...
 * VulkanDebug::endDebugLabel(device, cmdBuffer);
 * @endcode
 */
namespace VulkanDebug {

/**
 * @brief Creates a debug messenger for validation layers
 * @param instance Valid Vulkan instance
 * @param pCreateInfo Pointer to debug messenger creation info
 * @param pAllocator Optional custom allocator callbacks
 * @param pDebugMessenger Pointer to store the created debug messenger handle
 * @return VK_SUCCESS on success, error code otherwise
 * @throws std::runtime_error if debug messenger creation fails
 *
 * Example:
 * @code
 * VkDebugUtilsMessengerCreateInfoEXT createInfo{};
 * populateDebugMessengerCreateInfo(createInfo);
 * VkDebugUtilsMessengerEXT debugMessenger;
 * if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger)
 *     != VK_SUCCESS) {
 *     throw std::runtime_error("Failed to create debug messenger");
 * }
 * @endcode
 */
VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger);

/**
 * @brief Destroys a debug messenger
 * @param instance Vulkan instance that created the debug messenger
 * @param debugMessenger Debug messenger handle to destroy
 * @param pAllocator Optional custom allocator callbacks
 *
 * Example:
 * @code
 * destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
 * @endcode
 */
void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator);

/**
 * @brief Populates debug messenger creation info with default settings
 * @param createInfo Reference to creation info structure to populate
 *
 * Default configuration:
 * - All message severities (verbose, info, warning, error)
 * - All message types (general, validation, performance)
 * - Standard debug callback function
 *
 * Example:
 * @code
 * VkDebugUtilsMessengerCreateInfoEXT createInfo{};
 * populateDebugMessengerCreateInfo(createInfo);
 * // createInfo is now ready for debug messenger creation
 * @endcode
 */
void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo);

/**
 * @brief Checks if requested validation layers are available
 * @param validationLayers Vector of validation layer names to check
 * @return true if all requested layers are available, false otherwise
 *
 * Example:
 * @code
 * std::vector<const char*> layers = {
 *     "VK_LAYER_KHRONOS_validation"
 * };
 * if (!checkValidationLayerSupport(layers)) {
 *     throw std::runtime_error("Validation layers not available");
 * }
 * @endcode
 */
bool checkValidationLayerSupport(
    const std::vector<const char*>& validationLayers);

/**
 * @brief Debug callback function for validation layer messages
 * @param messageSeverity Severity level of the message
 * @param messageType Type of the message
 * @param pCallbackData Pointer to struct containing message details
 * @param pUserData User data passed to callback
 * @return VK_FALSE to continue execution, VK_TRUE to abort
 *
 * Message severities:
 * - VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic info
 * - VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational messages
 * - VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Warning messages
 * - VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Error messages
 *
 * Message types:
 * - VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: General messages
 * - VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Validation messages
 * - VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Performance warnings
 */
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

/**
 * @brief Sets a debug name for a Vulkan object
 * @param device Logical device that created the object
 * @param objectType Type of the Vulkan object
 * @param object Handle of the object (cast to uint64_t)
 * @param name Debug name to assign
 *
 * Example:
 * @code
 * // Name a buffer
 * setDebugObjectName(device, VK_OBJECT_TYPE_BUFFER,
 *                    (uint64_t)vertexBuffer, "MainVertexBuffer");
 *
 * // Name a texture
 * setDebugObjectName(device, VK_OBJECT_TYPE_IMAGE,
 *                    (uint64_t)textureImage, "AlbedoTexture");
 *
 * // Name a pipeline
 * setDebugObjectName(device, VK_OBJECT_TYPE_PIPELINE,
 *                    (uint64_t)pipeline, "MainRenderPipeline");
 * @endcode
 */
void setDebugObjectName(
    VkDevice device,
    VkObjectType objectType,
    uint64_t object,
    const std::string& name);

/**
 * @brief Begins a labeled debug region in a command buffer
 * @param device Logical device
 * @param commandBuffer Command buffer to insert label into
 * @param labelName Name of the debug region
 * @param color RGBA color for the region in debugging tools
 *
 * Example:
 * @code
 * float color[4] = {0.0f, 1.0f, 0.0f, 1.0f};  // Green
 * beginDebugLabel(device, cmdBuffer, "Shadow Pass", color);
 * // Record shadow pass commands...
 * endDebugLabel(device, cmdBuffer);
 * @endcode
 */
void beginDebugLabel(
    VkDevice device,
    VkCommandBuffer commandBuffer,
    const std::string& labelName,
    const float color[4]);

/**
 * @brief Ends a labeled debug region in a command buffer
 * @param device Logical device
 * @param commandBuffer Command buffer containing the region
 *
 * @note Must be paired with a corresponding beginDebugLabel call
 */
void endDebugLabel(
    VkDevice device,
    VkCommandBuffer commandBuffer);

/**
 * @brief Inserts a single debug label in a command buffer
 * @param device Logical device
 * @param commandBuffer Command buffer to insert label into
 * @param labelName Name of the debug marker
 * @param color RGBA color for the marker in debugging tools
 *
 * Example:
 * @code
 * float color[4] = {1.0f, 1.0f, 0.0f, 1.0f};  // Yellow
 * insertDebugLabel(device, cmdBuffer, "Draw Skybox", color);
 * // Record skybox drawing commands...
 * @endcode
 */
void insertDebugLabel(
    VkDevice device,
    VkCommandBuffer commandBuffer,
    const std::string& labelName,
    const float color[4]);

} // namespace VulkanDebug

} // namespace ev 