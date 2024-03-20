#include <beet_gfx/gfx_utils.h>
#include <beet_gfx/gfx_types.h>

#include <beet_shared/assert.h>

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