#ifndef BEETROOT_GFX_BUFFER_H
#define BEETROOT_GFX_BUFFER_H

#include <vulkan/vulkan_core.h>

//===PUBLIC_STRUCTS=====================================================================================================
struct GfxBuffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor = {};
    VkDeviceSize size = 0;
    VkDeviceSize alignment = 0;
    void *mappedData = nullptr;

    VkBufferUsageFlags usageFlags = {};
    VkMemoryPropertyFlags memoryPropertyFlags = {};
};
//======================================================================================================================

//===API================================================================================================================
void gfx_buffer_copy_immediate(GfxBuffer &src, GfxBuffer &dst, VkQueue queue, VkBufferCopy *copyRegion);

VkResult gfx_buffer_create(const VkBufferUsageFlags &usageFlags,
                           const VkMemoryPropertyFlags &memoryPropertyFlags,
                           GfxBuffer &outBuffer,
                           const VkDeviceSize size,
                           void *inData);

VkResult gfx_buffer_create(const VkBufferUsageFlags &usageFlags,
                           const VkMemoryPropertyFlags &memoryPropertyFlags,
                           const VkDeviceSize &size,
                           VkBuffer &outBuffer,
                           VkDeviceMemory &memory,
                           void *inData);
//======================================================================================================================

#endif //BEETROOT_GFX_BUFFER_H
