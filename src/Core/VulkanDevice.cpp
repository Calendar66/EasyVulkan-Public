#include "EasyVulkan/Core/VulkanDevice.hpp"
#include <stdexcept>
#include <set>
#include <string>

namespace ev {

namespace {
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        #ifdef __APPLE__
        "VK_KHR_portability_subset"
        #endif
    };
}

VulkanDevice::VulkanDevice(VkInstance instance, 
                          const VkPhysicalDeviceFeatures* deviceFeatures,
                          const std::vector<const char*>* additionalExtensions)
    : m_instance(instance)
    , m_physicalDevice(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE)
    , m_graphicsQueue(VK_NULL_HANDLE)
    , m_computeQueue(VK_NULL_HANDLE)
    , m_transferQueue(VK_NULL_HANDLE) {
    
#if !defined(OHOS)
    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }
#endif

    // Store device features if provided
    if (deviceFeatures) {
        m_deviceFeatures = *deviceFeatures;
    }

    // Store additional extensions if provided
    if (additionalExtensions) {
        m_additionalExtensions = *additionalExtensions;
    }
}

VulkanDevice::~VulkanDevice() {
    if (m_allocator != VK_NULL_HANDLE) {
        vmaDestroyAllocator(m_allocator);
        m_allocator = VK_NULL_HANDLE;
    }
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
    if (m_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
#if !defined(OHOS)
    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
#endif
}

#if !defined(OHOS)
void VulkanDevice::createWindow(uint32_t width, uint32_t height, const char* title) {
    // Tell GLFW not to create an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Disable window resizing for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        throw std::runtime_error("Failed to create GLFW window!");
    }
}
#endif

#if !defined(__OHOS__)
void VulkanDevice::createSurface() {
    if (m_window == nullptr) {
        throw std::runtime_error("Window must be created before surface!");
    }

    VkResult result = glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}
#else
void VulkanDevice::createSurfaceOHOS(OHNativeWindow* window) {
    if (window == nullptr) {
        throw std::runtime_error("OHOS Window must be created before surface!");
    }
    m_window = window;

    VkSurfaceCreateInfoOHOS surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_SURFACE_CREATE_INFO_OHOS;
    surfaceCreateInfo.window = window;
    int err = vkCreateSurfaceOHOS(m_instance, &surfaceCreateInfo, NULL, &m_surface);
    if (err != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}
#endif




#if !defined(__OHOS__)
void VulkanDevice::initialize(uint32_t width, uint32_t height, bool enableMemoryBudget) {
    // Create window and surface first as they are needed for device selection
    createWindow(width, height, "EasyVulkan");
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    setupAllocator(enableMemoryBudget);
}
#else
void VulkanDevice::initializeOHOS(uint32_t width, uint32_t height, bool enableMemoryBudget,OHNativeWindow* window) {
    // Create window and surface first as they are needed for device selection
    createSurfaceOHOS(window);
    pickPhysicalDevice();
    createLogicalDevice();
    setupAllocator(enableMemoryBudget);
}
#endif


void VulkanDevice::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    // Pick the first suitable device
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            m_queueFamilyIndices = findQueueFamilies(device);
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void VulkanDevice::createLogicalDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        m_queueFamilyIndices.graphicsFamily,
        m_queueFamilyIndices.computeFamily,
        m_queueFamilyIndices.transferFamily
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Combine required and additional extensions
    std::vector<const char*> extensions = deviceExtensions;
    extensions.insert(extensions.end(), 
                     m_additionalExtensions.begin(), 
                     m_additionalExtensions.end());



    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &m_deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;


    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    // Get queue handles
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.computeFamily, 0, &m_computeQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.transferFamily, 0, &m_transferQueue);
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        
        // Prefer discrete GPUs
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            return true;
        }
    }

    return indices.isComplete() && extensionsSupported;
}

bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // First try to find dedicated queues
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        const auto& queueFamily = queueFamilies[i];
        
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && !indices.hasGraphics) {
            indices.graphicsFamily = i;
            indices.hasGraphics = true;
        }

        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT && !indices.hasCompute) {
            indices.computeFamily = i;
            indices.hasCompute = true;
        }

        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && !indices.hasTransfer) {
            indices.transferFamily = i;
            indices.hasTransfer = true;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    // If we couldn't find dedicated queues, use graphics queue for compute/transfer
    if (!indices.hasCompute) {
        indices.computeFamily = indices.graphicsFamily;
        indices.hasCompute = indices.hasGraphics;
    }
    if (!indices.hasTransfer) {
        indices.transferFamily = indices.graphicsFamily;
        indices.hasTransfer = indices.hasGraphics;
    }

    return indices;
}

std::vector<const char*> VulkanDevice::getRequiredDeviceExtensions() {
    return deviceExtensions;
}

void VulkanDevice::setupAllocator(bool enableMemoryBudget) {
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = m_physicalDevice;
    allocatorInfo.device = m_device;
    allocatorInfo.instance = m_instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    
    // Enable memory budget extension for better memory usage tracking
    if(enableMemoryBudget) {
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    }

    if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS) {
        throw std::runtime_error("failed to create VMA allocator!");
    }
}

} // namespace ev 