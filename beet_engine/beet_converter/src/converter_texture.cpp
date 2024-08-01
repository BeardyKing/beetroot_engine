#include <beet_converter/converter_interface.h>
#include <beet_converter/converter_types.h>

#include <beet_shared/log.h>
#include <beet_shared/assert.h>
#include <beet_shared/filesystem.h>
#include <beet_shared/memory.h>
#include <beet_shared/c_string.h>

//===INTERNAL_STRUCTS===================================================================================================
extern ConverterLocations g_converterLocations;
//======================================================================================================================

std::string texture_format_to_string(const TextureFormat textureFormat) {
    switch (textureFormat) {
        case TextureFormat::RGBA8:
            return "RGBA8";
        case TextureFormat::RGBA16:
            return "RGBA16";

        case TextureFormat::BC1RGBA:
            return "BC1RGBA";
        case TextureFormat::BC2:
            return "BC2";
        case TextureFormat::BC3:
            return "BC3";
        case TextureFormat::BC4:
            return "BC4";
        case TextureFormat::BC5:
            return "BC5";
        case TextureFormat::BC6H:
            return "BC6H";
        case TextureFormat::BC7:
            return "BC7";
        default: SANITY_CHECK();
    };
    SANITY_CHECK();
    return "ERR_UNKNOWN_FORMAT";
}

constexpr char CONVERTER_MAX_MIP_COUNT[] = "-miplevels 20";
constexpr char CONVERTER_ENCODE_TYPE[] = "-EncodeWith GPU";
constexpr char CONVERTER_THREAD_COUNT[] = "-EncodeWith GPU";
constexpr char CONVERTER_2X2_MIPS[] = "-mipsize 2";

//===API================================================================================================================
bool convert_texture_dds(const char *localAssetDir, const char *inFileFormat, const TextureFormat format) {
    const std::string inPath = std::format("{}{}{}", g_converterLocations.rawAssetDir, localAssetDir, inFileFormat);
    const std::string outPath = std::format("{}{}.dds", g_converterLocations.targetAssetDir, localAssetDir);

    const std::string cmd = std::format(
            "{} -fd {} {} {} {} {} {} {}",
            g_converterLocations.compressonatorCLI,
            texture_format_to_string(format),
            CONVERTER_THREAD_COUNT,
            CONVERTER_ENCODE_TYPE,
            false ? CONVERTER_2X2_MIPS : "", // GPU encoders work on 4x4 which isn't ideal as we don't get as many mip maps. likely toggle on HIGH quality settings
            CONVERTER_MAX_MIP_COUNT,
            inPath,
            outPath
    );

    if (!fs_file_exists(inPath.c_str())) {
        return false;
    }

    if (converter_cache_check_needs_convert(outPath.c_str(), inPath.c_str())) {
        log_info(MSG_CONVERTER, "texture: %s \n", outPath.c_str());
        system(cmd.c_str());
    }
    return true;
}
//======================================================================================================================
