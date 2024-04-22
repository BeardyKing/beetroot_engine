#include <beet_converter/converter_interface.h>
#include <beet_converter/converter_types.h>

#include <beet_shared/assert.h>
#include <beet_shared/log.h>

#include <string>

//===INTERNAL_STRUCTS===================================================================================================
ConverterFileLocations g_converterDirs;
ConverterOptions g_converterOptions;
//======================================================================================================================

//===API================================================================================================================
bool converter_init(const char *rawAssetDir, const char *targetAssetDir) {
    g_converterDirs.rawAssetDir = rawAssetDir;
    g_converterDirs.targetAssetDir = targetAssetDir;

    if (g_converterDirs.rawAssetDir.empty() || g_converterDirs.targetAssetDir.empty()) {
        SANITY_CHECK();
        return false;
    }
    return true;
};

void converter_option_set_ignore_cache(const bool ignoreCacheOption) {
    g_converterOptions.ignoreConvertCache = ignoreCacheOption;
    if (g_converterOptions.ignoreConvertCache) {
        log_info(MSG_CONVERTER, "Option set - ignoring converter cache.\n");
    }
}
//======================================================================================================================