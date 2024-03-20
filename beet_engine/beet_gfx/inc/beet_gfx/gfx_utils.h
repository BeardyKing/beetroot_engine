#ifndef BEETROOT_GFX_UTILS_H
#define BEETROOT_GFX_UTILS_H

#include <cstdint>
#include <vulkan/vulkan_core.h>

uint32_t gfx_utils_get_memory_type(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);

#endif //BEETROOT_GFX_UTILS_H
