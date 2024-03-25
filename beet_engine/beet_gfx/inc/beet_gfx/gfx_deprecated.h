#ifndef BEETROOT_GFX_DEPRECATED_H
#define BEETROOT_GFX_DEPRECATED_H

#include <vulkan/vulkan_core.h>

void gfx_create_deprecated_frame_buffer();
void gfx_cleanup_deprecated_frame_buffer();

void gfx_create_deprecated_render_pass();
void gfx_cleanup_deprecated_render_pass();

void gfx_deprecated_render_pass(VkCommandBuffer &cmdBuffer);

#endif //BEETROOT_GFX_DEPRECATED_H
