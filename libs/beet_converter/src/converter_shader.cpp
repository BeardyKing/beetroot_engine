#include <beet_converter/converter_interface.h>
#include <beet_converter/converter_types.h>

#include <beet_shared/log.h>
#include <beet_shared/assert.h>
#include <beet_shared/filesystem.h>

//===INTERNAL_STRUCTS===================================================================================================
extern ConverterLocations g_converterLocations;
//======================================================================================================================

//===API================================================================================================================
bool convert_shader_spv(const char *localAssetDir) {
    const std::string inPath = std::format("{}{}", g_converterLocations.rawAssetDir, localAssetDir);
    const std::string outPath = std::format("{}{}", g_converterLocations.targetAssetDir, localAssetDir);
    const std::string cmd = std::format("{} {} {} {}", g_converterLocations.glslValidatorCLI, "-V -o", outPath, inPath);
    fs_mkdir_recursive(outPath.c_str());

    if (!fs_file_exists(inPath.c_str())) {
        return false;
    }

    if (converter_cache_check_needs_convert(outPath.c_str(), inPath.c_str())) {
        log_info(MSG_CONVERTER, "shader: %s \n", outPath.c_str());
        system(cmd.c_str());
    }
    return true;
}
//======================================================================================================================