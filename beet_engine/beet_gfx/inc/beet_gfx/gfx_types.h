#ifndef BEETROOT_GFX_TYPES_H
#define BEETROOT_GFX_TYPES_H

#include <vulkan/vulkan_core.h>

#include <beet_gfx/gfx_vulkan_platform_defines.h>

#include <beet_math/mat4.h>
#include <beet_math/vec3.h>
#include <beet_math/vec2.h>
#include <vector>

struct SwapChainBuffers {
    VkImage image;
    VkImageView view;
};

struct DepthImage {
    VkImage image;
    VkDeviceMemory deviceMemory;
    VkImageView view;
};

struct Semaphores {
    VkSemaphore presentDone;
    VkSemaphore renderDone;
};

struct VulkanSwapChain {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    uint32_t imageCount;
    VkImage *images = {nullptr};
    SwapChainBuffers *buffers = {nullptr};

    // TODO: consider this being a ptr to some global size instead
    uint32_t width = 1280;
    uint32_t height = 720;
};

struct QueueFamilyIndices {
    uint32_t graphics = {UINT32_MAX};
    uint32_t compute = {UINT32_MAX};
    uint32_t transfer = {UINT32_MAX};
    uint32_t present = {UINT32_MAX};
};

struct GfxBuffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor;
    VkDeviceSize size = 0;
    VkDeviceSize alignment = 0;
    void *mappedData = nullptr;

    VkBufferUsageFlags usageFlags;
    VkMemoryPropertyFlags memoryPropertyFlags;
};

struct UniformData {
    glm::mat4 projection;
    glm::mat4 view;
};

struct VulkanBackend {
    VkInstance instance = {VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice = {VK_NULL_HANDLE};
    VkDevice device = {VK_NULL_HANDLE};
    VkQueue queue = {VK_NULL_HANDLE};

    VkCommandPool graphicsCommandPool = {VK_NULL_HANDLE};
    VkCommandBuffer *graphicsCommandBuffers = {}; // size is based off swapChain.imageCount
    VkRenderPass renderPass = {VK_NULL_HANDLE};
    VkFence *graphicsFenceWait = {};// size is based off swapChain.imageCount

    DepthImage depthStencil = {};

    VkExtensionProperties *supportedExtensions = {};
    uint32_t extensionsCount = {};
    VkLayerProperties *supportedValidationLayers = {};
    uint32_t validationLayersCount = {};

    VulkanSwapChain swapChain = {};
    VkFramebuffer *frameBuffers = {}; // size is based off swapChain.imageCount
    uint32_t currentBufferIndex = 0;

    Semaphores semaphores = {};
    VkPipelineStageFlags submitPipelineStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = {};

    QueueFamilyIndices queueFamilyIndices = {};
    VkPipelineCache pipelineCache = {VK_NULL_HANDLE};

    VkPhysicalDeviceProperties deviceProperties = {};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};

    VkPipeline cubePipeline{VK_NULL_HANDLE};

    uint32_t objectCount = 0;

    //===INDIRECT===================
    //REPLACE WITH POOL
    std::vector<VkDrawIndexedIndirectCommand> indirectCommands;
    // Contains the indirect drawing commands
    GfxBuffer indirectCommandsBuffer;
    uint32_t indirectDrawCount{0};
    //==============================

    //===UNIFORM BUFFER=============
    GfxBuffer uniformBuffer = {VK_NULL_HANDLE};
    //==============================

    VkPipelineLayout pipelineLayout = {VK_NULL_HANDLE};
    VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout = {VK_NULL_HANDLE};

    VkCommandBuffer immediateCommandBuffer = {VK_NULL_HANDLE};
};

struct GfxTexture {
    VkImage image;
    VkDeviceMemory deviceMemory;
    VkImageView view;
    VkImageLayout layout;
    uint32_t imageSamplerType;
};


#endif //BEETROOT_GFX_TYPES_H