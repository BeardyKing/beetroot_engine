#ifndef BEETROOT_GFX_TYPES_H
#define BEETROOT_GFX_TYPES_H

#include <vulkan/vulkan_core.h> // must go before any vulkan implementations

#include <beet_math/mat4.h>
#include <beet_math/vec3.h>
#include <beet_math/vec2.h>

#include <beet_gfx/gfx_vulkan_platform_defines.h>
#include <beet_gfx/gfx_buffer.h> // TODO: consider moving the struct into here instead.
#include <beet_gfx/gfx_mesh.h> // TODO: consider moving the struct into here instead.

#include <beet_shared/texture_formats.h>

#include <vector>

//===GPU_TYPES========================================================================================================
//TODO: add infra to include this type in shader code.
struct SceneUBO {
    mat4 projection;
    mat4 view;
    vec3f position;
    float unused_0;
};

struct LinePoint3D {
    // used in line & triangle strip shaders
    vec3f position;
    uint32_t color;
};
//======================================================================================================================

//===PUBLIC_STRUCTS=====================================================================================================
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

    uint32_t imageCount = {0};
    uint32_t currentImageIndex = {0};
    uint32_t lastImageIndex = {0};
    VkImage images[BEET_SWAP_CHAIN_IMAGE_MAX] = {nullptr};
    SwapChainBuffers buffers[BEET_SWAP_CHAIN_IMAGE_MAX] = {nullptr};

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

struct TargetVulkanFormats {
    // TODO: this struct should only last a single "frame" would be a good candidate for the inevitable gfx backend arena allocator
    VkSurfaceFormatKHR surfaceFormat = {};
    VkFormat depthFormat = {};
    VkFormat colorFormat = {};
};

struct GfxImageBuffer {
    VkImage image;
    VkImageView view;
    VkDeviceMemory deviceMemory;
};

struct VulkanBackend {
    VkInstance instance = {VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice = {VK_NULL_HANDLE};
    VkDevice device = {VK_NULL_HANDLE};
    VkQueue queue = {VK_NULL_HANDLE};

    VkCommandPool graphicsCommandPool = {VK_NULL_HANDLE};
    VkCommandBuffer graphicsCommandBuffers[BEET_BUFFER_COUNT] = {VK_NULL_HANDLE};

    GfxImageBuffer depthStencilBuffer = {};
    GfxImageBuffer colorBuffer = {};

    GfxImageBuffer resolvedDepthBuffer; // multisample resolve.

    VkExtensionProperties *supportedExtensions = {};
    uint32_t extensionsCount = {};
    VkLayerProperties *supportedValidationLayers = {};
    uint32_t validationLayersCount = {};

    VkFence graphicsFenceWait[BEET_SWAP_CHAIN_IMAGE_MAX] = {VK_NULL_HANDLE};
    Semaphores semaphores[BEET_SWAP_CHAIN_IMAGE_MAX] = {VK_NULL_HANDLE};
    VulkanSwapChain swapChain = {};

    QueueFamilyIndices queueFamilyIndices = {};
    VkPipelineCache pipelineCache = {VK_NULL_HANDLE};

    VkPhysicalDeviceProperties deviceProperties = {};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};


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
    VkSampleCountFlagBits sampleCount = {VK_SAMPLE_COUNT_1_BIT};
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
#if BEET_DEBUG
    char debug_name[MAX_PATH];
    TextureFormat debug_textureFormat;
    uint32_t debug_mipMapCount;
    uint32_t debug_width;
    uint32_t debug_height;
    uint32_t debug_depth;
#endif
};
//======================================================================================================================

#endif //BEETROOT_GFX_TYPES_H