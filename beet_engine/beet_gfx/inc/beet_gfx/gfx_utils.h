#ifndef BEETROOT_GFX_UTILS_H
#define BEETROOT_GFX_UTILS_H

#include <cstdint>
#include <vulkan/vulkan_core.h>

uint32_t gfx_utils_get_memory_type(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);

VkSurfaceFormatKHR gfx_utils_select_surface_format();
VkFormat gfx_utils_find_depth_format(const VkImageTiling &desiredTilingFormat);

#endif //BEETROOT_GFX_UTILS_H
