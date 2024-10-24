#ifndef BEETROOT_CONVERTER_INTERFACE_H
#define BEETROOT_CONVERTER_INTERFACE_H

#include <beet_shared/texture_formats.h>

//===INIT_&_SHUTDOWN====================================================================================================
bool converter_init(const char *rawAssetDir, const char *targetAssetDir);
//======================================================================================================================

//===API================================================================================================================
bool convert_shader_spv(const char *localAssetDir);
bool convert_texture_dds(const char *localAssetDir, const char *inFileFormat, TextureFormat format);

bool converter_cache_check_needs_convert(const char *toPath, const char *fromPath);
void converter_option_set_ignore_cache(bool ignoreCacheOption);
//======================================================================================================================

#endif //BEETROOT_CONVERTER_INTERFACE_H
