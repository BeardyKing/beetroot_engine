#ifndef BEETROOT_GFX_UTILS_H
#define BEETROOT_GFX_UTILS_H

#include <beet_shared/texture_formats.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>

uint32_t gfx_utils_get_memory_type(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);

VkSurfaceFormatKHR gfx_utils_select_surface_format();

VkFormat gfx_utils_find_depth_format(const VkImageTiling &desiredTilingFormat);

VkFormat gfx_utils_beet_image_format_to_vk(TextureFormat textureFormat);
#endif //BEETROOT_GFX_UTILS_H
