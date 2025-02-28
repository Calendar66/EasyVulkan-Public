#include <EasyVulkan/Builders/BufferBuilder.hpp>
#include <EasyVulkan/Builders/CommandBufferBuilder.hpp>
#include <EasyVulkan/Builders/FramebufferBuilder.hpp>
#include <EasyVulkan/Builders/GraphicsPipelineBuilder.hpp>
#include <EasyVulkan/Builders/RenderPassBuilder.hpp>
#include <EasyVulkan/Builders/ShaderModuleBuilder.hpp>
#include <EasyVulkan/Core/CommandPoolManager.hpp>
#include <EasyVulkan/Core/ResourceManager.hpp>
#include <EasyVulkan/Core/SwapchainManager.hpp>
#include <EasyVulkan/Core/SynchronizationManager.hpp>
#include <EasyVulkan/Core/VulkanContext.hpp>
#include <EasyVulkan/Core/VulkanDevice.hpp>

#include <array>

// Vertex data structure
struct Vertex {
  float pos[2];
  float color[3];
};

class TriangleApp {
public:
  void run() {
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  uint32_t currentFrame = 0;
  std::unique_ptr<ev::VulkanContext> context;
  ev::VulkanDevice *device;
  ev::ResourceManager *resourceManager;
  ev::CommandPoolManager *cmdPoolManager;
  ev::SwapchainManager *swapchainManager;
  ev::SynchronizationManager *syncManager;

  VkRenderPass renderPass;
  VkPipeline graphicsPipeline;
  VkPipelineLayout pipelineLayout;
  VkBuffer vertexBuffer;
  std::vector<VkFramebuffer> framebuffers;
  std::vector<VkCommandBuffer> commandBuffers;

  void initVulkan() {
    // Initialize context
    context = std::make_unique<ev::VulkanContext>(true);
    context->initialize(800, 600);

    // Get device and managers
    device = context->getDevice();
    resourceManager = context->getResourceManager();
    cmdPoolManager = context->getCommandPoolManager();
    swapchainManager = context->getSwapchainManager();
    syncManager = context->getSynchronizationManager();

    // Create window and swapchain
    
    swapchainManager->createSwapchain();

    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createVertexBuffer();
    createCommandBuffers();

    // Setup frame synchronization
    syncManager->createFrameSynchronization(2); // Double buffering
  }

  void createRenderPass() {
    auto &builder = resourceManager->createRenderPass();

    // Color attachment
    builder.addColorAttachment(
        swapchainManager->getSwapchainImageFormat(), VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Single subpass
    builder.beginSubpass().addColorReference(0).endSubpass();

    renderPass = builder.build("main-render-pass");
  }

  void createGraphicsPipeline() {
    // Create vertex shader module
    auto vertShader = resourceManager->createShaderModule()
                          .loadFromFile("shaders/triangle.vert.spv")
                          .build("vertex-shader");

    // Create fragment shader module
    auto fragShader = resourceManager->createShaderModule()
                          .loadFromFile("shaders/triangle.frag.spv")
                          .build("fragment-shader");

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkCreatePipelineLayout(device->getLogicalDevice(), &pipelineLayoutInfo,
                           nullptr, &pipelineLayout);

    // Create graphics pipeline
    auto &builder = resourceManager->createGraphicsPipeline();

    // Add shader stages
    builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertShader)
        .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader);

    // Configure vertex input
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(Vertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributes{};
    // Position attribute
    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[0].offset = offsetof(Vertex, pos);
    // Color attribute
    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[1].offset = offsetof(Vertex, color);

    builder
        .setVertexInputState({bindingDesc},
                             {attributes.begin(), attributes.end()})
        .setInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .setViewport(VkViewport{
            0.0f,    // x
            0.0f,    // y
            800.0f,  // width
            600.0f,  // height
            0.0f,    // minDepth
            1.0f     // maxDepth
            })
        .setScissor(VkRect2D{
            {0, 0},
            {800, 600}
        })
        .setRasterizationState()
        .setMultisampleState()
        .setColorBlendState({VkPipelineColorBlendAttachmentState{
            VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA,
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
            VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}})
        .setLayout(pipelineLayout)
        .setRenderPass(renderPass);

    graphicsPipeline = builder.build("graphics-pipeline");
  }

  void createFramebuffers() {
    const auto &swapchainImageViews =
        swapchainManager->getSwapchainImageViews();

    framebuffers.resize(swapchainImageViews.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
      auto &builder = resourceManager->createFramebuffer();
      framebuffers[i] = builder.addAttachment(swapchainImageViews[i])
          .setDimensions(800, 600)
          .build(renderPass, "framebuffer-" + std::to_string(i));
    }
  }

  void createVertexBuffer() {
    const std::vector<Vertex> vertices = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                          {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                          {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

    auto &builder = resourceManager->createBuffer();
    vertexBuffer =
        builder.setSize(sizeof(vertices[0]) * vertices.size())
            .setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            .setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .buildAndInitialize(vertices.data(),
                                sizeof(vertices[0]) * vertices.size(),
                                "vertex-buffer");
  }

  void createCommandBuffers() {
    auto commandPool = cmdPoolManager->createCommandPool(
        device->getGraphicsQueueFamily(),
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    auto &builder = resourceManager->createCommandBuffer();
    commandBuffers = builder.setCommandPool(commandPool)
                         .setCount(framebuffers.size())
                         .buildMultiple();

    // Record command buffers
    for (size_t i = 0; i < commandBuffers.size(); i++) {
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

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
      vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                        graphicsPipeline);

      VkDeviceSize offset = 0;
      vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, &offset);
      vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

      vkCmdEndRenderPass(commandBuffers[i]);
      vkEndCommandBuffer(commandBuffers[i]);
    }
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(device->getWindow())) {
      std::cout << "Main loop" << std::endl;
      glfwPollEvents();
      drawFrame();
    }

    vkDeviceWaitIdle(device->getLogicalDevice());
  }

  void drawFrame() {
    // Wait for previous frame
    VkFence inFlightFence = syncManager->getInFlightFence(currentFrame);
    vkWaitForFences(device->getLogicalDevice(), 1, &inFlightFence, VK_TRUE,
                    UINT64_MAX);

    // Acquire next swapchain image
    uint32_t imageIndex = swapchainManager->acquireNextImage(
        syncManager->getImageAvailableSemaphore(currentFrame));

    // Reset fence for next frame
    vkResetFences(device->getLogicalDevice(), 1, &inFlightFence);

    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        syncManager->getImageAvailableSemaphore(currentFrame)};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {
        syncManager->getRenderFinishedSemaphore(currentFrame)};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, inFlightFence);

    // Present the image
    swapchainManager->presentImage(
        imageIndex, syncManager->getRenderFinishedSemaphore(currentFrame));

    currentFrame = (currentFrame + 1) % 2;
  }

  void cleanup() {
    vkDestroyPipelineLayout(device->getLogicalDevice(), pipelineLayout,
                            nullptr);
  }
};

int main() {
  TriangleApp app;

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}