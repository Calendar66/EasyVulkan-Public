cmake_minimum_required(VERSION 3.20)
project(EasyVulkan VERSION 0.1.0)

# ------------------------------------------------------------------------------
# Global Configuration
# ------------------------------------------------------------------------------
# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# Platform-Specific Configuration
# ------------------------------------------------------------------------------
if(WIN32)
    # Define platform-specific macros for Vulkan and Windows
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    add_definitions(-DNOMINMAX)

    # Set paths for GLFW (Windows) and Vulkan SDK
    set(GLFW_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/glfw-win/lib-vc2022")
    set(GLFW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/glfw-win/include")
    set(VULKAN_SDK_PATH "D:/vulkan")
    
    # Choose the appropriate Vulkan lib directory based on pointer size
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/Lib")
    else()
        set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/Lib32")
    endif()
else()
    # Set paths for GLFW (non-Windows) and Vulkan SDK (macOS/Unix)
    set(GLFW_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/glfw/lib-arm64")
    set(GLFW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/glfw/include")
    set(VULKAN_SDK_PATH "/Users/calendar1/Documents/zgz_mac/Vulkan/macOS")
    set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/lib")
endif()

# Add the library directories (affects all targets in this directory)
link_directories(${GLFW_LIB_DIR})
link_directories(${VULKAN_LIB_DIR})

# ------------------------------------------------------------------------------
# Source Files
# ------------------------------------------------------------------------------
# Collect source and header files for the project
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/EasyVulkan/*.hpp")

# ------------------------------------------------------------------------------
# Library Target Definition
# ------------------------------------------------------------------------------
add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Set include directories for the target (PUBLIC makes them propagate to dependents)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${VULKAN_SDK_PATH}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/VulkanMemoryAllocator/include
    ${GLFW_INCLUDE_DIR}
)

# ------------------------------------------------------------------------------
# Linking
# ------------------------------------------------------------------------------
if(WIN32)
    # Link Windows-specific libraries (using full paths)
    target_link_libraries(${PROJECT_NAME} PUBLIC 
        "${GLFW_LIB_DIR}/glfw3.lib"
        "${VULKAN_LIB_DIR}/vulkan-1.lib"
    )
else()
    # Link non-Windows libraries (assuming static GLFW and Vulkan library)
    target_link_libraries(${PROJECT_NAME} PUBLIC 
        "${GLFW_LIB_DIR}/libglfw3.a"
        "${VULKAN_LIB_DIR}/vulkan"
    )
endif()

# For macOS, link necessary system frameworks
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC 
        "-framework Cocoa" 
        "-framework IOKit" 
        "-framework CoreFoundation"
    )
endif()

# ------------------------------------------------------------------------------
# Precompiled Headers
# ------------------------------------------------------------------------------
target_precompile_headers(${PROJECT_NAME} PRIVATE <EasyVulkan/Common.hpp>)

# ------------------------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------------------------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)

# ------------------------------------------------------------------------------
# Installation Rules
# ------------------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
)