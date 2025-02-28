cmake_minimum_required(VERSION 3.20)
project(EasyVulkanTriangle)

# Set C++ standard to match main project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Add executable
add_executable(Triangle main.cpp)

# Link libraries
target_link_libraries(Triangle PRIVATE EasyVulkan)


# Compile shaders
find_program(GLSL_VALIDATOR glslangValidator REQUIRED)

if(NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found! Please install Vulkan SDK.")
endif()

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

set(SHADERS
    ${SHADER_SOURCE_DIR}/triangle.vert
    ${SHADER_SOURCE_DIR}/triangle.frag
)

foreach(SHADER ${SHADERS})
    get_filename_component(FILENAME ${SHADER} NAME)
    add_custom_command(
        OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
        COMMAND ${GLSL_VALIDATOR} -V ${SHADER} -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILENAME}"
    )
    list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_dependencies(Triangle shaders)

# Copy shaders to build directory
add_custom_command(
    TARGET Triangle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Triangle>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADER_BINARY_DIR} $<TARGET_FILE_DIR:Triangle>/shaders
) 