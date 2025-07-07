#include "EasyVulkan/Core/VulkanContext.hpp"
#include "EasyVulkan/Core/VulkanDevice.hpp"
#include "EasyVulkan/Core/SwapchainManager.hpp"
#include "EasyVulkan/Core/CommandPoolManager.hpp"
#include "EasyVulkan/Core/ResourceManager.hpp"
#include "EasyVulkan/Core/SynchronizationManager.hpp"
#ifdef __APPLE__
#include <vulkan/vulkan_metal.h>
#endif
#include <stdexcept>
#include <vector>
#include <iostream>

namespace ev {

namespace {
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

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

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger) {
        
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator) {
        
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
        
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
}

VulkanContext::VulkanContext(bool enableValidationLayers)
    : m_instance(VK_NULL_HANDLE)
    , m_debugMessenger(VK_NULL_HANDLE)
    , m_enableValidationLayers(enableValidationLayers)
    , m_instanceExtensions({}) {
}

VulkanContext::~VulkanContext() {
    cleanup();
}

#if !defined(__OHOS__)
void VulkanContext::initialize(uint32_t width, uint32_t height) {
    // Create instance
    createInstance();
    
    // Setup debug callbacks
    if (m_enableValidationLayers) {
        setupDebugCallbacks();
    }

    // Create window,curface and device
    m_device = std::make_unique<VulkanDevice>(m_instance, &m_deviceFeatures, &m_deviceExtensions);
    if(std::find(m_instanceExtensions.begin(), m_instanceExtensions.end(), "VK_KHR_get_physical_device_properties2") != m_instanceExtensions.end()) {
        m_device->initialize(width, height, true);
    } else {
        m_device->initialize(width, height,false);
    }

    // Create managers
    m_commandPoolManager = std::make_unique<CommandPoolManager>(m_device.get());
    m_resourceManager = std::make_unique<ResourceManager>(m_device.get(),this);
    m_synchronizationManager = std::make_unique<SynchronizationManager>(m_device.get());
    m_swapchainManager = std::make_unique<SwapchainManager>(m_device.get(),m_device->getSurface());
}
#else
void VulkanContext::initializeOHOS(uint32_t width, uint32_t height,OHNativeWindow* window) {
    // Create instance
    createInstance();
    
    // Setup debug callbacks
    if (m_enableValidationLayers) {
        setupDebugCallbacks();
    }

    // Create window,curface and device
    m_device = std::make_unique<VulkanDevice>(m_instance, &m_deviceFeatures, &m_deviceExtensions);
    if(std::find(m_instanceExtensions.begin(), m_instanceExtensions.end(), "VK_KHR_get_physical_device_properties2") != m_instanceExtensions.end()) {
        m_device->initializeOHOS(width, height, true,window);
    } else {
        m_device->initializeOHOS(width, height,false,window);
    }

    // Create managers
    m_commandPoolManager = std::make_unique<CommandPoolManager>(m_device.get());
    m_resourceManager = std::make_unique<ResourceManager>(m_device.get(),this);
    m_synchronizationManager = std::make_unique<SynchronizationManager>(m_device.get());
    m_swapchainManager = std::make_unique<SwapchainManager>(m_device.get(),m_device->getSurface());
}
#endif

void VulkanContext::cleanup() {
    // Cleanup managers first
    m_synchronizationManager.reset();
    m_resourceManager.reset();
    m_commandPoolManager.reset();
    m_swapchainManager.reset();
    m_device.reset();

    if (m_enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

void VulkanContext::createInstance() {
    if (m_enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    auto extensions = getRequiredExtensions();
    if (!checkInstanceExtensionSupport(extensions)) {
        throw std::runtime_error("some requested instance extensions are not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "EasyVulkan Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "EasyVulkan";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        std::string errorMsg = "failed to create instance! Error code: " + std::to_string(result);
        throw std::runtime_error(errorMsg);
    }
}

void VulkanContext::setupDebugCallbacks() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

bool VulkanContext::checkValidationLayerSupport() {
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

bool VulkanContext::checkInstanceExtensionSupport(const std::vector<const char*>& extensions) {
    uint32_t extensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    for (const char* extensionName : extensions) {
        bool extensionFound = false;

        for (const auto& extensionProperties : availableExtensions) {
            if (strcmp(extensionName, extensionProperties.extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }

        if (!extensionFound) {
            std::cerr << "Warning: Extension " << extensionName << " not found!" << std::endl;
            return false;
        }
    }

    return true;
}

std::vector<const char*> VulkanContext::getRequiredExtensions() {
    std::vector<const char*> extensions;

    // Add required surface extensions
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    
    #ifdef __APPLE__
    extensions.push_back("VK_EXT_metal_surface");
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    #elif defined(_WIN32)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    #elif defined(__OHOS__)
    extensions.push_back(VK_OHOS_SURFACE_EXTENSION_NAME);
    #endif

    if (m_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Add custom instance extensions
    for (const auto& extension : m_instanceExtensions) {
        extensions.push_back(extension);
    }

    return extensions;
}

void VulkanContext::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

} // namespace ev 