#include <beet_converter/converter_interface.h>
#include <beet_converter/converter_types.h>

#include <beet_shared/assert.h>
#include <beet_shared/log.h>

#include <string>

//===INTERNAL_STRUCTS===================================================================================================
ConverterLocations g_converterLocations;
ConverterOptions g_converterOptions;
//======================================================================================================================

//===API================================================================================================================
bool converter_init(const char *rawAssetDir, const char *targetAssetDir) {
    g_converterLocations.rawAssetDir = rawAssetDir;
    g_converterLocations.targetAssetDir = targetAssetDir;
    g_converterLocations.compressonatorSDKDir = getenv("COMPRESSONATOR_ROOT");
    g_converterLocations.vulkanSDKDir = getenv("VULKAN_SDK");

    g_converterLocations.compressonatorCLI = std::format("{}{}", g_converterLocations.compressonatorSDKDir, "\\bin\\CLI\\compressonatorcli.exe");
    g_converterLocations.glslValidatorCLI = std::format("{}{}", g_converterLocations.vulkanSDKDir, "\\Bin\\glslangValidator.exe");

    if (g_converterLocations.rawAssetDir.empty() ||
        g_converterLocations.targetAssetDir.empty() ||
        g_converterLocations.compressonatorSDKDir.empty() ||
        g_converterLocations.vulkanSDKDir.empty()) {
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