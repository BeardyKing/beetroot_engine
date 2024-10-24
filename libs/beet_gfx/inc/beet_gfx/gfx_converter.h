#ifndef BEETROOT_GFX_CONVERTER_H
#define BEETROOT_GFX_CONVERTER_H

#include <beet_shared/feature_defines.h>

#define FEATURE_CONVERT_ON_DEMAND FEATURE_ON

#if CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)
//===API================================================================================================================
void gfx_converter_init(const char *rawAssetDir, const char *targetAssetDir);
bool gfx_convert_shader_spv(const char *localAssetPath);
bool gfx_convert_texture_dds(const char *localAssetPath);
//======================================================================================================================
#endif //CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)

#endif //BEETROOT_GFX_CONVERTER_H
