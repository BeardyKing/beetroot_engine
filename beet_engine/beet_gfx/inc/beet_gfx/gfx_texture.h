#ifndef BEETROOT_GFX_TEXTURE_H
#define BEETROOT_GFX_TEXTURE_H

#include <vulkan/vulkan_core.h>
#include <beet_gfx/gfx_types.h>

void gfx_texture_create_immediate_dds(const char *path, GfxTexture &inOutTexture);
void gfx_texture_cleanup(GfxTexture &gfxTexture);
#endif //BEETROOT_GFX_TEXTURE_H
