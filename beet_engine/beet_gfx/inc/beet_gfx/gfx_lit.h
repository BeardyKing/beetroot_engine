#ifndef BEETROOT_GFX_LIT_H
#define BEETROOT_GFX_LIT_H

#include <beet_gfx/gfx_types.h>
#include <vulkan/vulkan_core.h>

void gfx_create_lit();
void gfx_cleanup_lit();

void gfx_lit_draw(VkCommandBuffer &cmdBuffer);

void gfx_lit_update_material_descriptor(VkDescriptorSet &outDescriptorSet, const GfxTexture &albedoTexture);
#endif //BEETROOT_GFX_LIT_H
