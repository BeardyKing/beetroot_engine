#ifndef BEETROOT_CONVERTER_INTERFACE_H
#define BEETROOT_CONVERTER_INTERFACE_H

//===INIT_&_SHUTDOWN====================================================================================================
bool converter_init(const char *rawAssetDir, const char *targetAssetDir);
//======================================================================================================================

//===API================================================================================================================
bool convert_shader_spv(const char *localAssetDir);

bool converter_cache_check_needs_convert(const char *toPath, const char *fromPath);
void converter_option_set_ignore_cache(bool ignoreCacheOption);
//======================================================================================================================

#endif //BEETROOT_CONVERTER_INTERFACE_H
