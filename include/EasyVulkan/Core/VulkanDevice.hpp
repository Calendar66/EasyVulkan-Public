/**
 * @file VulkanDevice.hpp
 * @brief Device management class for EasyVulkan framework
 * @details This file contains the VulkanDevice class which handles physical device selection,
 *          logical device creation, queue management, and memory allocation.
 */

#pragma once

#include "../Common.hpp"
#include <GLFW/glfw3.h>

namespace ev {

/**
 * @class VulkanDevice
 * @brief Manages Vulkan device creation and queue management
 * @details VulkanDevice is responsible for:
 *          - Physical device (GPU) selection
 *          - Logical device creation
 *          - Queue family management
 *          - Memory allocation via VMA
 *          - Window and surface management
 * 
 * @note Inheritance:
 *       - Override pickPhysicalDevice() for custom GPU selection logic
 *       - Override createLogicalDevice() to enable additional features/extensions
 */
class VulkanDevice {
public:
    /**
     * @brief Constructor for VulkanDevice
     * @param instance Valid Vulkan instance handle
     * @param deviceFeatures Optional physical device features to enable
     * @param additionalExtensions Optional additional device extensions to enable
     * @throws std::runtime_error if instance is VK_NULL_HANDLE
     */
    explicit VulkanDevice(VkInstance instance, 
                         const VkPhysicalDeviceFeatures* deviceFeatures = nullptr,
                         const std::vector<const char*>* additionalExtensions = nullptr);
    
    /**
     * @brief Virtual destructor for proper cleanup
     * @details Destroys the logical device, memory allocator, surface, and window
     */
    virtual ~VulkanDevice();

    /**
     * @brief Get the graphics queue handle
     * @return VkQueue Graphics queue handle
     */
    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }

    /**
     * @brief Get the graphics queue family index
     * @return uint32_t Graphics queue family index
     */
    uint32_t getGraphicsQueueFamily() const { return m_queueFamilyIndices.graphicsFamily; }

    /**
     * @brief Get the compute queue handle
     * @return VkQueue Compute queue handle
     */
    VkQueue getComputeQueue() const { return m_computeQueue; }

    /**
     * @brief Get the compute queue family index
     * @return uint32_t Compute queue family index
     */
    uint32_t getComputeQueueFamily() const { return m_queueFamilyIndices.computeFamily; }


    /**
     * @brief Get the transfer queue handle
     * @return VkQueue Transfer queue handle
     */
    VkQueue getTransferQueue() const { return m_transferQueue; }

    /**
     * @brief Get the physical device handle
     * @return VkPhysicalDevice Physical device (GPU) handle
     */
    VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

    /**
     * @brief Get the logical device handle
     * @return VkDevice Logical device handle
     */
    VkDevice getLogicalDevice() const { return m_device; }

    /**
     * @brief Get the VMA allocator handle
     * @return VmaAllocator Memory allocator handle
     */
    VmaAllocator getAllocator() const { return m_allocator; }

    /**
     * @brief Get the window handle
     * @return GLFWwindow* Window handle
     */
    GLFWwindow* getWindow() const { return m_window; }

    /**
     * @brief Get the surface handle
     * @return VkSurfaceKHR Surface handle
     */
    VkSurfaceKHR getSurface() const { return m_surface; }

    /**
     * @brief Initializes the device by selecting physical device and creating logical device
     * @param width Initial window width
     * @param height Initial window height
     * @throws std::runtime_error if device creation fails
     * @details This method:
     *          1. Selects a suitable physical device
     *          2. Creates a logical device with required queues
     *          3. Sets up the memory allocator
     */
    virtual void initialize(uint32_t width, uint32_t height);

    /**
     * @brief Create a window with the specified dimensions
     * @param width Window width
     * @param height Window height
     * @param title Window title
     */
    void createWindow(uint32_t width, uint32_t height, const char* title);

protected:
    /**
     * @brief Selects a suitable physical device (GPU)
     * @throws std::runtime_error if no suitable device is found
     * @details Default implementation selects the first GPU that:
     *          - Supports required extensions
     *          - Has required queue families
     *          - Meets minimum feature requirements
     */
    virtual void pickPhysicalDevice();

    /**
     * @brief Creates the logical device with required queues and features
     * @throws std::runtime_error if device creation fails
     * @details Sets up:
     *          - Required device extensions
     *          - Queue creation
     *          - Device features
     */
    virtual void createLogicalDevice();

    /**
     * @struct QueueFamilyIndices
     * @brief Stores queue family indices for different queue types
     */
    struct QueueFamilyIndices {
        uint32_t graphicsFamily;    ///< Graphics queue family index
        uint32_t computeFamily;     ///< Compute queue family index
        uint32_t transferFamily;    ///< Transfer queue family index
        bool hasGraphics = false;   ///< Whether graphics queue family was found
        bool hasCompute = false;    ///< Whether compute queue family was found
        bool hasTransfer = false;   ///< Whether transfer queue family was found

        /**
         * @brief Check if all required queue families are available
         * @return true if all required queue families are found
         */
        bool isComplete() const {
            return hasGraphics && hasCompute && hasTransfer;
        }
    };

    /**
     * @brief Create the Vulkan surface for the window
     */
    virtual void createSurface();

private:
    VkInstance m_instance;                  ///< Vulkan instance handle
    VkPhysicalDevice m_physicalDevice;      ///< Physical device handle
    VkDevice m_device;                      ///< Logical device handle
    VmaAllocator m_allocator;              ///< Memory allocator handle

    VkQueue m_graphicsQueue;               ///< Graphics queue handle
    VkQueue m_computeQueue;                ///< Compute queue handle
    VkQueue m_transferQueue;               ///< Transfer queue handle

    QueueFamilyIndices m_queueFamilyIndices; ///< Queue family indices

    GLFWwindow* m_window{nullptr};      ///< GLFW window handle
    VkSurfaceKHR m_surface{VK_NULL_HANDLE}; ///< Vulkan surface handle

    /**
     * @brief Check if a physical device meets requirements
     * @param device Physical device to check
     * @return true if device is suitable
     */
    bool isDeviceSuitable(VkPhysicalDevice device);

    /**
     * @brief Check if device supports required extensions
     * @param device Physical device to check
     * @return true if all required extensions are supported
     */
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    /**
     * @brief Find queue families that support required operations
     * @param device Physical device to check
     * @return QueueFamilyIndices structure with found indices
     */
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    /**
     * @brief Set up the VMA allocator
     * @throws std::runtime_error if allocator creation fails
     */
    void setupAllocator();

    /**
     * @brief Get list of required device extensions
     * @return Vector of required extension names
     */
    std::vector<const char*> getRequiredDeviceExtensions();

    // Device customization options
    VkPhysicalDeviceFeatures m_deviceFeatures{};
    std::vector<const char*> m_additionalExtensions;
};

} // namespace ev 