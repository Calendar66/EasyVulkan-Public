#include "EasyVulkan/Utils/VulkanDebug.hpp"
#include <iostream>
#include <stdexcept>

namespace ev {
namespace VulkanDebug {

VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkCreateDebugUtilsMessengerEXT");
    
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkDestroyDebugUtilsMessengerEXT");
    
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

bool checkValidationLayerSupport(
    const std::vector<const char*>& validationLayers) {
    
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData)
{
    // Only print warnings or worse
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

        // Print any queue labels (if you use vkQueueBeginDebugUtilsLabelEXT)
        for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
            const auto& lbl = pCallbackData->pQueueLabels[i];
            std::cerr << "\t[QueueLabel] " << lbl.pLabelName << std::endl;
        }

        // Print any command-buffer labels (if you use vkCmdBeginDebugUtilsLabelEXT)
        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
            const auto& lbl = pCallbackData->pCmdBufLabels[i];
            std::cerr << "\t[CmdBufLabel] " << lbl.pLabelName << std::endl;
        }

        // Print all objects with their debug names (aliases)
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
            const auto& obj = pCallbackData->pObjects[i];
            std::cerr << "\t[Object] Type: " 
                      << obj.objectType 
                      << ", Handle: 0x" << std::hex << obj.objectHandle << std::dec 
                      << ", Name: " 
                      << (obj.pObjectName ? obj.pObjectName : "N/A")
                      << std::endl;
        }
    }

    // Returning VK_FALSE tells Vulkan that we do NOT want to abort the call
    return VK_FALSE;
}

void setDebugObjectName(
    VkDevice device,
    VkObjectType objectType,
    uint64_t object,
    const std::string& name) {
    
    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = objectType;
    nameInfo.objectHandle = object;
    nameInfo.pObjectName = name.c_str();

    auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(
        device,
        "vkSetDebugUtilsObjectNameEXT");
    
    if (func != nullptr) {
        if (func(device, &nameInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to set debug object name!");
        }
    }
}

void beginDebugLabel(
    VkDevice device,
    VkCommandBuffer commandBuffer,
    const std::string& labelName,
    const float color[4]) {
    
    VkDebugUtilsLabelEXT labelInfo{};
    labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    labelInfo.pLabelName = labelName.c_str();
    memcpy(labelInfo.color, color, sizeof(float) * 4);

    auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(
        device,
        "vkCmdBeginDebugUtilsLabelEXT");
    
    if (func != nullptr) {
        func(commandBuffer, &labelInfo);
    }
}

void endDebugLabel(
    VkDevice device,
    VkCommandBuffer commandBuffer) {
    
    auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(
        device,
        "vkCmdEndDebugUtilsLabelEXT");
    
    if (func != nullptr) {
        func(commandBuffer);
    }
}

void insertDebugLabel(
    VkDevice device,
    VkCommandBuffer commandBuffer,
    const std::string& labelName,
    const float color[4]) {
    
    VkDebugUtilsLabelEXT labelInfo{};
    labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    labelInfo.pLabelName = labelName.c_str();
    memcpy(labelInfo.color, color, sizeof(float) * 4);

    auto func = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetDeviceProcAddr(
        device,
        "vkCmdInsertDebugUtilsLabelEXT");
    
    if (func != nullptr) {
        func(commandBuffer, &labelInfo);
    }
}

} // namespace VulkanDebug
} // namespace ev 