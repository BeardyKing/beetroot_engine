#include <beet_gfx/gfx_utils.h>
#include <beet_gfx/gfx_types.h>

#include <beet_shared/assert.h>
#include <beet_shared/memory.h>

#include <vulkan/vulkan.h>

extern VulkanBackend g_vulkanBackend;

uint32_t gfx_utils_get_memory_type(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < g_vulkanBackend.deviceMemoryProperties.memoryTypeCount; i++) {
        if ((memoryTypeBits & 1) == 1) {
            if ((g_vulkanBackend.deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        memoryTypeBits >>= 1;
    }
    ASSERT_MSG(false, "Err: fid not find memory type of target %u", properties);
    return UINT32_MAX; // TODO:TEST: validate this is the correct value to return to crash the backend.
}

VkSurfaceFormatKHR gfx_utils_select_surface_format() {
    uint32_t surfaceFormatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &surfaceFormatsCount, nullptr);

    VkSurfaceFormatKHR *surfaceFormats = (VkSurfaceFormatKHR *) mem_zalloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkanBackend.physicalDevice, g_vulkanBackend.swapChain.surface, &surfaceFormatsCount, surfaceFormats);

    // try select best surface format
    VkSurfaceFormatKHR selectedSurfaceFormat{};
    if ((surfaceFormatsCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
        selectedSurfaceFormat = BEET_TARGET_SWAPCHAIN_FORMAT;
    } else {
        for (uint32_t i = 0; i < surfaceFormatsCount; ++i) {
            if ((surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM) &&
                (surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
                selectedSurfaceFormat = surfaceFormats[i];
                break;
            }
        }
    }
    mem_free(surfaceFormats);
    return selectedSurfaceFormat;
}

VkFormat gfx_utils_find_depth_format(const VkImageTiling &desiredTilingFormat) {
    constexpr VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    constexpr uint32_t FORMAT_CANDIDATE_COUNT = 5;
    constexpr VkFormat candidates[FORMAT_CANDIDATE_COUNT]{
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM,
    };

    for (uint32_t i = 0; i < FORMAT_CANDIDATE_COUNT; ++i) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(g_vulkanBackend.physicalDevice, candidates[i], &props);

        if ((desiredTilingFormat == VK_IMAGE_TILING_LINEAR) &&
            ((props.linearTilingFeatures & features) == features)) {
            return candidates[i];
        }
        if ((desiredTilingFormat == VK_IMAGE_TILING_OPTIMAL) &&
            ((props.optimalTilingFeatures & features) == features)) {
            return candidates[i];
        }
    }
    ASSERT_MSG(false, "Err: could not find supported depth format");
    return VK_FORMAT_UNDEFINED;
}