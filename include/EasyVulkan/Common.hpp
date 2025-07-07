/**
 * @file Common.hpp
 * @brief Common definitions and utilities for EasyVulkan framework
 * @details This file contains:
 *          - Common type definitions and aliases
 *          - Framework-wide constants
 *          - Utility functions
 *          - Required includes for the framework
 */

#pragma once

// Vulkan related includes
#include <vulkan/vulkan.h>  ///< Core Vulkan header
#include <vk_mem_alloc.h>   ///< Vulkan Memory Allocator

// STL includes
#include <iostream>         ///< For standard I/O operations
#include <vector>          ///< For dynamic arrays
#include <string>          ///< For string handling
#include <memory>          ///< For smart pointers
#include <optional>        ///< For optional values
#include <array>           ///< For fixed-size arrays
#include <unordered_map>   ///< For hash maps
#include <set>             ///< For ordered unique collections
#include <algorithm>       ///< For STL algorithms
#include <stdexcept>       ///< For standard exceptions

#if defined(__OHOS__)
#include <ace/xcomponent/native_interface_xcomponent.h>
#include <native_window/external_window.h>
#else
#include <GLFW/glfw3.h>    ///< For GLFW - only included on non-OpenHarmony platforms
#endif


// Data structure
#include <EasyVulkan/DataStructures.hpp>

/**
 * @namespace ev
 * @brief Main namespace for EasyVulkan framework
 * @details Contains all classes, functions, and utilities provided by the framework.
 *          The namespace is abbreviated as 'ev' for brevity and clarity.
 */
namespace ev {

/**
 * @typedef String
 * @brief Alias for std::string
 * @details Used throughout the framework for string operations
 */
using String = std::string;

/**
 * @typedef Vector
 * @brief Template alias for std::vector
 * @tparam T Type of elements in the vector
 * @details Used throughout the framework for dynamic arrays
 *
 * Example:
 * @code
 * ev::Vector<float> vertices;
 * ev::Vector<VkDescriptorSet> descriptorSets;
 * @endcode
 */
template<typename T>
using Vector = std::vector<T>;

/**
 * @typedef Optional
 * @brief Template alias for std::optional
 * @tparam T Type of the optional value
 * @details Used for values that may or may not be present
 *
 * Example:
 * @code
 * ev::Optional<uint32_t> queueFamilyIndex;
 * if (queueFamilyIndex.has_value()) {
 *     // Use the value
 * }
 * @endcode
 */
template<typename T>
using Optional = std::optional<T>;

/**
 * @typedef UniquePtr
 * @brief Template alias for std::unique_ptr
 * @tparam T Type of the managed object
 * @details Used for exclusive ownership of resources
 *
 * Example:
 * @code
 * ev::UniquePtr<Buffer> vertexBuffer = std::make_unique<Buffer>();
 * @endcode
 */
template<typename T>
using UniquePtr = std::unique_ptr<T>;

/**
 * @typedef SharedPtr
 * @brief Template alias for std::shared_ptr
 * @tparam T Type of the managed object
 * @details Used for shared ownership of resources
 *
 * Example:
 * @code
 * ev::SharedPtr<Texture> texture = std::make_shared<Texture>();
 * @endcode
 */
template<typename T>
using SharedPtr = std::shared_ptr<T>;

/**
 * @def VULKAN_API_VERSION
 * @brief Defines the Vulkan API version used by the framework
 * @details Currently set to Vulkan 1.2
 *
 * This constant is used when:
 * - Creating a Vulkan instance
 * - Checking device compatibility
 * - Enabling version-specific features
 */
constexpr uint32_t VULKAN_API_VERSION = VK_API_VERSION_1_2;

/**
 * @brief Utility function to check Vulkan result codes
 * @param result The VkResult to check
 * @param message Error message to include if the result is not VK_SUCCESS
 * @throws std::runtime_error if result is not VK_SUCCESS
 *
 * Example:
 * @code
 * VkResult result = vkCreateBuffer(device, &createInfo, nullptr, &buffer);
 * ThrowIfFailed(result, "Failed to create buffer");
 * @endcode
 *
 * Common usage:
 * - After creating Vulkan objects
 * - After allocating resources
 * - After submitting commands
 * - After any Vulkan operation that returns VkResult
 */
inline void ThrowIfFailed(VkResult result, const char* message) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(message);
    }
}

/**
 * @enum LogLevel
 * @brief Defines different severity levels for logging
 */
enum class LogLevel {
    Debug,      ///< Debug-level information for development
    Info,       ///< General information about program execution
    Warning,    ///< Warnings that don't prevent execution but might indicate problems
    Error       ///< Serious errors that might lead to program failure
};

/**
 * @brief Get string representation of a log level
 * @param level The LogLevel to convert
 * @return String representation of the log level
 */
inline const char* LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "[EasyVulkan][DEBUG]";
        case LogLevel::Info:    return "[EasyVulkan][INFO]";
        case LogLevel::Warning: return "[EasyVulkan][WARNING]";
        case LogLevel::Error:   return "[EasyVulkan][ERROR]";
        default:               return "[EasyVulkan][UNKNOWN]";
    }
}

/**
 * @brief Base logging function that handles all log levels
 * @param level The severity level of the log message
 * @param message The message to log
 * @param file The source file where the log was called from
 * @param line The line number where the log was called from
 */
inline void Log(LogLevel level, const String& message, const char* file = nullptr, int line = -1) {
    std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;
    out << LogLevelToString(level) << " ";
    if (file && line != -1) {
        out << "[" << file << ":" << line << "] ";
    }
    out << message << std::endl;
}

/**
 * @brief Log a debug message
 * @param message The debug message to log
 * @param file The source file (automatically filled)
 * @param line The line number (automatically filled)
 */
inline void LogDebug(const String& message, const char* file = __FILE__, int line = __LINE__) {
    Log(LogLevel::Debug, message, file, line);
}

/**
 * @brief Log an informational message
 * @param message The info message to log
 */
inline void LogInfo(const String& message) {
    Log(LogLevel::Info, message);
}

/**
 * @brief Log a warning message
 * @param message The warning message to log
 * @param file The source file (automatically filled)
 * @param line The line number (automatically filled)
 */
inline void LogWarning(const String& message, const char* file = __FILE__, int line = __LINE__) {
    Log(LogLevel::Warning, message, file, line);
}

/**
 * @brief Log an error message
 * @param message The error message to log
 * @param file The source file (automatically filled)
 * @param line The line number (automatically filled)
 */
inline void LogError(const String& message, const char* file = __FILE__, int line = __LINE__) {
    Log(LogLevel::Error, message, file, line);
}

} // namespace ev 