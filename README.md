# EasyVulkan

EasyVulkan is a modern C++ framework designed to simplify Vulkan development by providing a high-level, intuitive API while maintaining the performance benefits of Vulkan. It offers a builder-based approach to Vulkan object creation and management, making Vulkan development more accessible and less error-prone.

## 🚀 What's New in This Update

### Major New Features
- **🎯 Automatic Debug Object Naming**: All resources are automatically named for enhanced debugging with RenderDoc, NSight Graphics, and validation layers
- **🧠 Advanced Memory Management**: Comprehensive VMA integration with memory monitoring, budget tracking, and defragmentation
- **📱 OpenHarmony Platform Support**: Full native support for HarmonyOS applications with `initializeOHOS()` method
- **🔍 Enhanced Debug Utilities**: Command buffer labels, GPU profiling markers, and improved validation callbacks
- **⚡ Memory Leak Resolution**: Fixed builder lifetime management by returning builders by value instead of reference

### Interface Changes
- **Builder Pattern Update**: Changed from `auto &builder = resourceManager->createX()` to `auto builder = resourceManager->createX()` (prevents memory leaks)
- **OpenHarmony Initialization**: New `initializeOHOS(width, height, nativeWindow)` method for HarmonyOS
- **Debug Integration**: Automatic `setDebugObjectName()` calls for all registered resources

### New APIs Added
- `resourceManager->getMemoryUsage()` - Get detailed memory statistics
- `resourceManager->getMemoryBudget()` - Monitor memory budget per heap
- `resourceManager->defragmentMemory()` - Perform memory defragmentation
- `resourceManager->printMemoryUsage()` - Print memory usage to console
- `VulkanDebug::beginDebugLabel()` / `endDebugLabel()` - GPU profiling markers
- `VulkanDebug::insertDebugLabel()` - Single debug markers

## Features

- **Builder Pattern API**: Create Vulkan objects with a fluent, chainable API
- **Resource Management**: Automatic tracking and cleanup of Vulkan resources with enhanced lifecycle management
- **Automatic Debug Object Naming**: Resources are automatically named for enhanced debugging and profiling
- **Advanced Memory Management**: Memory monitoring, defragmentation, and budget tracking with VMA integration
- **Simplified Synchronization**: Easy-to-use synchronization primitives
- **Command Buffer Management**: Streamlined command buffer recording and submission with debug labels
- **Enhanced Debug Utilities**: Validation callbacks, command buffer profiling markers, and object naming
- **Cross-Platform Support**: Works on Windows, macOS, Linux, and OpenHarmony
- **Modern C++**: Utilizes C++20 features for clean, expressive code

## Motvation
When first learning Vulkan, we're often amazed by the greater freedom it offers developers compared to OpenGL, enabling much finer control over the rendering pipeline. However,**when using Vulkan in actual projects, even minor feature modifications can involve extensive code changes and considerable repetitive work**. During my winter break, I encapsulated some commonly used functions based on my year and a half of Vulkan experience. As the complexity increased, I developed it into a complete project—**EasyVulkan**.

## Requirements

- C++20 compatible compiler
- CMake 3.20 or higher
- Vulkan SDK 1.3.x or higher
- GLFW 3.3 or higher (not required for OpenHarmony)

### OpenHarmony Additional Requirements
- OpenHarmony SDK 3.0 or higher
- Native development tools for OpenHarmony

## Installation

### Clone the Repository

```bash
git clone https://github.com/yourusername/EasyVulkan.git
cd EasyVulkan
```

### Build with CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Platform-Specific Setup

#### Windows

Make sure to set the `VULKAN_SDK_PATH` in CMakeLists.txt to your Vulkan SDK installation path.

#### macOS

Install the Vulkan SDK for macOS from the [LunarG website](https://www.lunarg.com/vulkan-sdk/) and set the `VULKAN_SDK_PATH` in CMakeLists.txt.

#### Linux

Install the Vulkan SDK and development packages:

```bash
sudo apt-get install libvulkan-dev vulkan-tools
```

#### OpenHarmony

Install the OpenHarmony SDK and configure the native development environment. For OpenHarmony applications, use the `initializeOHOS()` method instead of `initialize()`:

```cpp
// OpenHarmony initialization
context->initializeOHOS(width, height, nativeWindow);
```

Note: OpenHarmony builds use different surface extensions and don't require GLFW.

## Quick Start: Triangle Example

The Triangle example demonstrates how to create a simple Vulkan application using EasyVulkan. Here's a step-by-step breakdown:

### 1. Initialize Vulkan Context

```cpp
#include <EasyVulkan/Core/VulkanContext.hpp>

// Create and initialize the Vulkan context
auto context = std::make_unique<ev::VulkanContext>(true); // Enable validation layers
context->initialize(800, 600); // Window dimensions

// Get device and managers
auto device = context->getDevice();
auto resourceManager = context->getResourceManager();
auto cmdPoolManager = context->getCommandPoolManager();
auto swapchainManager = context->getSwapchainManager();
auto syncManager = context->getSynchronizationManager();

// Create swapchain
swapchainManager->createSwapchain();
```

### 2. Create Render Pass

```cpp
#include <EasyVulkan/Builders/RenderPassBuilder.hpp>

// Create render pass using builder pattern
auto builder = resourceManager->createRenderPass();

// Add color attachment
builder.addColorAttachment(
    swapchainManager->getSwapchainImageFormat(), 
    VK_SAMPLE_COUNT_1_BIT,
    VK_ATTACHMENT_LOAD_OP_CLEAR, 
    VK_ATTACHMENT_STORE_OP_STORE,
    VK_IMAGE_LAYOUT_UNDEFINED, 
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

// Add subpass
builder.beginSubpass().addColorReference(0).endSubpass();

// Build the render pass
VkRenderPass renderPass = builder.build("main-render-pass");
```

### 3. Create Graphics Pipeline

```cpp
#include <EasyVulkan/Builders/ShaderModuleBuilder.hpp>
#include <EasyVulkan/Builders/GraphicsPipelineBuilder.hpp>

// Create shader modules
auto vertShader = resourceManager->createShaderModule()
                      .loadFromFile("shaders/triangle.vert.spv")
                      .build("vertex-shader");

auto fragShader = resourceManager->createShaderModule()
                      .loadFromFile("shaders/triangle.frag.spv")
                      .build("fragment-shader");

// Create pipeline layout
VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
vkCreatePipelineLayout(device->getLogicalDevice(), &pipelineLayoutInfo,
                       nullptr, &pipelineLayout);

// Create graphics pipeline
auto builder = resourceManager->createGraphicsPipeline();

// Configure pipeline with builder pattern
builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertShader)
    .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader)
    .setVertexInputState(bindingDescriptions, attributeDescriptions)
    .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    .setViewport(viewport)
    .setScissor(scissor)
    .setRasterizationState()
    .setMultisampleState()
    .setColorBlendState(colorBlendAttachments)
    .setLayout(pipelineLayout)
    .setRenderPass(renderPass);

// Build the pipeline
VkPipeline graphicsPipeline = builder.build("graphics-pipeline");
```

### 4. Create Vertex Buffer

```cpp
#include <EasyVulkan/Builders/BufferBuilder.hpp>

// Define vertex data
struct Vertex {
  float pos[2];
  float color[3];
};

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

// Create vertex buffer
auto builder = resourceManager->createBuffer();
VkBuffer vertexBuffer =
    builder.setSize(sizeof(vertices[0]) * vertices.size())
        .setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        .buildAndInitialize(vertices.data(),
                            sizeof(vertices[0]) * vertices.size(),
                            "vertex-buffer");
```

### 5. Create Command Buffers

```cpp
#include <EasyVulkan/Builders/CommandBufferBuilder.hpp>

// Create command pool
auto commandPool = cmdPoolManager->createCommandPool(
    device->getGraphicsQueueFamily(),
    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

// Create command buffers
auto builder = resourceManager->createCommandBuffer();
std::vector<VkCommandBuffer> commandBuffers = 
    builder.setCommandPool(commandPool)
         .setCount(framebuffers.size())
         .buildMultiple();

// Record command buffers
for (size_t i = 0; i < commandBuffers.size(); i++) {
    // Begin command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {800, 600};

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
    
    // Bind pipeline and vertex buffer
    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphicsPipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, &offset);
    
    // Draw triangle
    vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

    // End render pass and command buffer
    vkCmdEndRenderPass(commandBuffers[i]);
    vkEndCommandBuffer(commandBuffers[i]);
}
```

### 6. Main Loop and Drawing

```cpp
// Setup frame synchronization
syncManager->createFrameSynchronization(2); // Double buffering

// Main loop
while (!glfwWindowShouldClose(device->getWindow())) {
    glfwPollEvents();
    
    // Wait for previous frame
    VkFence inFlightFence = syncManager->getInFlightFence(currentFrame);
    vkWaitForFences(device->getLogicalDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    // Acquire next swapchain image
    uint32_t imageIndex = swapchainManager->acquireNextImage(
        syncManager->getImageAvailableSemaphore(currentFrame));

    // Reset fence for next frame
    vkResetFences(device->getLogicalDevice(), 1, &inFlightFence);

    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {syncManager->getImageAvailableSemaphore(currentFrame)};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {syncManager->getRenderFinishedSemaphore(currentFrame)};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, inFlightFence);

    // Present the image
    swapchainManager->presentImage(imageIndex, signalSemaphores, 1);

    // Update frame index
    currentFrame = (currentFrame + 1) % 2;
}

// Wait for device to finish operations
vkDeviceWaitIdle(device->getLogicalDevice());
```

## Project Structure

```
EasyVulkan/
├── include/                  # Public headers
│   └── EasyVulkan/
│       ├── Core/             # Core functionality
│       ├── Builders/         # Builder pattern implementations
│       └── Utils/            # Utility functions
├── src/                      # Implementation files
├── examples/                 # Example applications
│   └── Triangle/             # Simple triangle rendering example
├── docs/                     # Documentation
└── thirdParty/              # Third-party dependencies
```

## Core Components

### VulkanContext

The central class that initializes Vulkan and manages all other components. Supports both standard platforms and OpenHarmony with platform-specific initialization methods.

### ResourceManager

Manages Vulkan resources and provides builder objects for resource creation. Features automatic debug object naming, comprehensive resource tracking, and advanced memory management with VMA integration including monitoring and defragmentation capabilities.

### SwapchainManager

Handles swapchain creation, image acquisition, and presentation across all supported platforms.

### CommandPoolManager

Manages command pools and provides utilities for command buffer allocation with enhanced debugging support.

### SynchronizationManager

Provides utilities for synchronization primitives like semaphores and fences with frame synchronization management.

## Builder Classes

EasyVulkan uses the builder pattern to simplify Vulkan object creation:

- **GraphicsPipelineBuilder**: Create graphics pipelines
- **BufferBuilder**: Create and initialize buffers
- **ImageBuilder**: Create and initialize images
- **RenderPassBuilder**: Create render passes
- **FramebufferBuilder**: Create framebuffers
- **ShaderModuleBuilder**: Load and create shader modules
- **CommandBufferBuilder**: Allocate command buffers
- **DescriptorSetBuilder**: Create descriptor sets
- **SamplerBuilder**: Create samplers
- **ComputePipelineBuilder**: Create compute pipelines

## Advanced Features

### Automatic Debug Object Naming

EasyVulkan automatically assigns debug names to all created resources, enhancing debugging and profiling capabilities:

```cpp
// Resources are automatically named when created
auto buffer = resourceManager->createBuffer()
    .setSize(1024)
    .setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    .build("MyVertexBuffer");

// The debug name "MyVertexBuffer" is automatically set for validation layers
// and debugging tools like RenderDoc, NSight Graphics, etc.
```

### Memory Management and Monitoring

Advanced memory management features with VMA integration:

```cpp
// Get memory usage statistics
auto stats = resourceManager->getMemoryUsage();
std::cout << "Total allocated: " << stats.total.statistics.allocationBytes << " bytes\n";
std::cout << "Total allocations: " << stats.total.statistics.allocationCount << "\n";

// Print detailed memory information
resourceManager->printMemoryUsage(true);

// Get memory budget for each heap
auto budgets = resourceManager->getMemoryBudget();
for (size_t i = 0; i < budgets.size(); ++i) {
    float usage = (float)budgets[i].usage * 100.0f / (float)budgets[i].budget;
    std::cout << "Heap " << i << ": " << usage << "% used\n";
}

// Perform memory defragmentation (use sparingly)
auto defragStats = resourceManager->defragmentMemory();
std::cout << "Defragmentation moved " << defragStats.allocationsMoved << " allocations\n";
```

### Debug Utilities and Profiling

Enhanced debugging with command buffer labels and validation:

```cpp
#include <EasyVulkan/Utils/VulkanDebug.hpp>

// Use debug labels for GPU profiling
float color[4] = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
ev::VulkanDebug::beginDebugLabel(device, commandBuffer, "Shadow Pass", color);
// ... record shadow rendering commands ...
ev::VulkanDebug::endDebugLabel(device, commandBuffer);

// Insert debug markers
float yellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
ev::VulkanDebug::insertDebugLabel(device, commandBuffer, "Draw Skybox", yellow);
```

### Platform Abstraction

Seamless cross-platform development with conditional compilation:

```cpp
// Standard platforms (Windows, macOS, Linux)
context->initialize(width, height);

// OpenHarmony platform
#ifdef __OHOS__
context->initializeOHOS(width, height, nativeWindow);
#endif
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License.

## Acknowledgments

- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [GLFW](https://www.glfw.org/)
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
