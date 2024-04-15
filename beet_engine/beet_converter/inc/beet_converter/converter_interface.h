#ifndef BEETROOT_CONVERTER_INTERFACE_H
#define BEETROOT_CONVERTER_INTERFACE_H

//===INIT======================================================================
bool converter_init(const char *rawAssetDir, const char *targetAssetDir);
//=============================================================================

//===CONVERT===================================================================
bool convert_shader_spv(const char *localAssetDir);
//=============================================================================

//===OPTIONS===================================================================
void converter_option_set_ignore_cache(bool ignoreCacheOption);
//=============================================================================

//===CACHE=====================================================================
bool converter_cache_check_needs_convert(const char *toPath, const char *fromPath);
//=============================================================================

#endif //BEETROOT_CONVERTER_INTERFACE_H
