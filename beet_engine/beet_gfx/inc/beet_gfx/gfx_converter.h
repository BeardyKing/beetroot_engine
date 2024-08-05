#ifndef BEETROOT_GFX_CONVERTER_H
#define BEETROOT_GFX_CONVERTER_H

//TODO: will extend to EDITOR/RUNTIME instead of being forced on.
#define BEET_CONVERT_ON_DEMAND 1

#if BEET_CONVERT_ON_DEMAND
//===API================================================================================================================
void gfx_converter_init(const char *rawAssetDir, const char *targetAssetDir);
bool gfx_convert_shader_spv(const char *localAssetPath);
bool gfx_convert_texture_dds(const char *localAssetPath);
//======================================================================================================================
#endif //BEET_CONVERT_ON_DEMAND

#endif //BEETROOT_GFX_CONVERTER_H
