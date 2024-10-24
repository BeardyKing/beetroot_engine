#ifndef BEETROOT_GFX_VULKAN_SURFACE_H
#define BEETROOT_GFX_VULKAN_SURFACE_H

#include <vulkan/vulkan.h>

//===API================================================================================================================
void gfx_create_surface(void *windowHandle, const VkInstance *instance, VkSurfaceKHR *outSurface);
//======================================================================================================================

#endif //BEETROOT_GFX_VULKAN_SURFACE_H
