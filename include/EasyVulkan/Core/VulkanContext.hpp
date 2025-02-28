/**
 * @file VulkanContext.hpp
 * @brief Main context class for EasyVulkan framework, managing Vulkan instance and core managers
 */

#pragma once

#include "../Common.hpp"

#include <memory>
#include <vector>
#include <string>

namespace ev {

// Forward declarations
class VulkanDevice;
class SwapchainManager;
class CommandPoolManager;
class ResourceManager;
class SynchronizationManager;

/**
 * @brief VulkanContext is responsible for creating the Vulkan instance, 
 * managing validation layers, and owning essential managers like 
 * VulkanDevice, SwapchainManager, and ResourceManager.
 *
 * Inheritance: 
 * - This class can be subclassed to override how the instance and device 
 *   are created. 
 * - Methods such as initialize() are extension points for inserting 
 *   custom logic.
 */
class VulkanContext {
public:
    /**
     * @brief Constructor for VulkanContext
     * @param enableValidationLayers Whether to enable Vulkan validation layers
     */
    explicit VulkanContext(bool enableValidationLayers = true);
    
    /**
     * @brief Virtual destructor to ensure proper cleanup of derived classes
     */
    virtual ~VulkanContext();

    /**
     * @brief Sets the device features to be enabled during device creation
     * @param features Vulkan physical device features to enable
     * @note Must be called before initialize()
     */
    void setDeviceFeatures(const VkPhysicalDeviceFeatures& features) { m_deviceFeatures = features; }

    /**
     * @brief Sets additional device extensions to be enabled during device creation
     * @param extensions List of extension names to enable
     * @note Must be called before initialize()
     */
    void setDeviceExtensions(const std::vector<const char*>& extensions) { m_deviceExtensions = extensions; }

    /**
     * @brief Initializes the Vulkan instance, device, and associated managers
     * @param width Initial window width
     * @param height Initial window height
     * @throws std::runtime_error if initialization fails
     */
    virtual void initialize(uint32_t width, uint32_t height);

    // Getters for managers
    VulkanDevice* getDevice() const { return m_device.get(); }
    SwapchainManager* getSwapchainManager() const { return m_swapchainManager.get(); }
    ResourceManager* getResourceManager() const { return m_resourceManager.get(); }
    CommandPoolManager* getCommandPoolManager() const { return m_commandPoolManager.get(); }
    SynchronizationManager* getSynchronizationManager() const { return m_synchronizationManager.get(); }

    /**
     * @brief Cleans up all Vulkan resources
     */
    virtual void cleanup();

protected:
    /**
     * @brief Creates the Vulkan instance with required extensions and layers
     * @throws std::runtime_error if instance creation fails
     */
    virtual void createInstance();

    /**
     * @brief Sets up debug callbacks if validation layers are enabled
     * @throws std::runtime_error if debug callback setup fails
     */
    virtual void setupDebugCallbacks();

private:
    // Vulkan instance handle
    VkInstance m_instance;
    
    // Debug callback handle
    VkDebugUtilsMessengerEXT m_debugMessenger;
    
    // Whether validation layers are enabled
    bool m_enableValidationLayers;

    // Core managers
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<VkSurfaceKHR> m_surface;
    std::unique_ptr<SwapchainManager> m_swapchainManager;
    std::unique_ptr<CommandPoolManager> m_commandPoolManager;
    std::unique_ptr<ResourceManager> m_resourceManager;
    std::unique_ptr<SynchronizationManager> m_synchronizationManager;

    // Helper methods
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    // Device customization options
    VkPhysicalDeviceFeatures m_deviceFeatures{};
    std::vector<const char*> m_deviceExtensions;
};

} // namespace ev 