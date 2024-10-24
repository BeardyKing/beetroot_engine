#ifndef BEETROOT_GFX_SKY_H
#define BEETROOT_GFX_SKY_H

#include <beet_gfx/gfx_types.h>
#include <vulkan/vulkan_core.h>

//===API================================================================================================================
bool gfx_rebuild_sky_pipeline();
void gfx_sky_draw(VkCommandBuffer &cmdBuffer);
void gfx_sky_update_material_descriptor(VkDescriptorSet &outDescriptorSet, const GfxTexture &albedoTexture);
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void gfx_create_sky();
void gfx_cleanup_sky();
//======================================================================================================================

#endif //BEETROOT_GFX_SKY_H
