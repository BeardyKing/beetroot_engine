#ifndef BEETROOT_GFX_TYPES_H
#define BEETROOT_GFX_TYPES_H

#include <vulkan/vulkan_core.h> // must go before any vulkan implementations

#include <beet_gfx/gfx_vulkan_platform_defines.h>
#include <beet_math/mat4.h>
#include <beet_math/vec3.h>
#include <beet_math/vec2.h>

#include <beet_gfx/gfx_buffer.h> // TODO: consider moving the struct into here instead.
#include <beet_gfx/gfx_mesh.h> // TODO: consider moving the struct into here instead.

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
    VkFence *graphicsFenceWait = {};// size is based off swapChain.imageCount

    DepthImage depthStencil = {};

    VkExtensionProperties *supportedExtensions = {};
    uint32_t extensionsCount = {};
    VkLayerProperties *supportedValidationLayers = {};
    uint32_t validationLayersCount = {};

    VulkanSwapChain swapChain = {};

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

    GfxBuffer instanceBuffer;
    //==============================

    //===UNIFORM BUFFER=============
    GfxBuffer uniformBuffer = {VK_NULL_HANDLE};
    //==============================

    VkPipelineLayout indirectPipelineLayout = {VK_NULL_HANDLE};
    VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout = {VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool = {VK_NULL_HANDLE};

    VkCommandBuffer immediateCommandBuffer = {VK_NULL_HANDLE};

    //===DEPRECATED=================
    // variable deprecated due to dynamic rendering extension.
    // if dynamic rendering ext is not supported we should consider a fallback.
    VkRenderPass deprecated_renderPass = {VK_NULL_HANDLE};
    VkFramebuffer *deprecated_frameBuffers = {VK_NULL_HANDLE}; // size is based off swapChain.imageCount
    //==============================
};

//TODO: Add something like a GfxTextureArray which stores all the textures loaded from a package.
// this way we can have a single descriptor/image allocation to a large amount of textures
struct GfxTexture {
    VkImage image;
    VkDeviceMemory deviceMemory;
    VkImageView view;
    VkImageLayout layout;
    uint32_t imageSamplerType;
    VkDescriptorImageInfo descriptor;
};

struct GlobTextures {
    GfxTexture uvGrid;
};

struct GlobMeshes {
    GfxMesh cube;
};

#endif //BEETROOT_GFX_TYPES_H