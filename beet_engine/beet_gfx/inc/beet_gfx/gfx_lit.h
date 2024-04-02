#ifndef BEETROOT_GFX_LIT_H
#define BEETROOT_GFX_LIT_H

#include <vulkan/vulkan_core.h>

void gfx_create_lit();
void gfx_cleanup_lit();

void gfx_lit_draw(VkCommandBuffer &cmdBuffer);

#endif //BEETROOT_GFX_LIT_H
