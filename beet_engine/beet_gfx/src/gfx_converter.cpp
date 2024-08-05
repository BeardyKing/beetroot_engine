#include <beet_gfx/gfx_converter.h>
#include <beet_converter/converter_interface.h>

#include <beet_shared/assert.h>
#include <beet_shared/log.h>
#include <beet_shared/c_string.h>

constexpr uint32_t SUPPORTED_CONVERTER_FORMATS_COUNT = 3;
static constexpr const char *SUPPORTED_CONVERTER_FORMATS[SUPPORTED_CONVERTER_FORMATS_COUNT]{
    ".png",
    ".tga",
    ".exr",
};

#if BEET_CONVERT_ON_DEMAND
//===API================================================================================================================
void gfx_converter_init(const char *rawAssetDir, const char *targetAssetDir) {

    const bool initResult = converter_init(rawAssetDir, targetAssetDir);
    ASSERT(initResult)

    const bool optionForceReconvert = false;
    converter_option_set_ignore_cache(optionForceReconvert);

    log_info(MSG_GFX, "Using on-demand converter lib\n");
    log_info(MSG_GFX, "Source dir: %s\n", rawAssetDir);
    log_info(MSG_GFX, "Target dir: %s\n", targetAssetDir);
    log_info(MSG_GFX, "Shader compile : [TRUE]\n");
    log_info(MSG_GFX, "texture convert : [TRUE]\n");
    log_info(MSG_GFX, "Option force recompile : [%s]\n", optionForceReconvert ? "TRUE" : "FALSE");
}

bool gfx_convert_shader_spv(const char *localAssetPath) {
    const bool compileResult = convert_shader_spv(localAssetPath);
    ASSERT_MSG(compileResult, "Err: gfx failed to compile shader %s \n", localAssetPath);
    return compileResult;
}

#include <beet_shared/filesystem.h>
#include <beet_converter/converter_types.h>

extern ConverterLocations g_converterLocations;

bool gfx_convert_texture_dds(const char* localAssetPath){
    const char* delim = (c_str_search_reverse(localAssetPath, "."));
    if(!c_str_search_reverse(localAssetPath, ".")){
        SANITY_CHECK();
    }
    size_t copySize = delim - localAssetPath;
    char fileNameNoExt[128] = {};
    memcpy(fileNameNoExt, localAssetPath, copySize);

    char searchFileName[128] = {};
    uint32_t foundIndex = {};
    bool foundFile = false;
    for (uint32_t i = 0; i < SUPPORTED_CONVERTER_FORMATS_COUNT; ++i) {
        memset(searchFileName, 0, sizeof(char) * 128);

        sprintf(searchFileName, "%s%s%s", g_converterLocations.rawAssetDir.c_str(), fileNameNoExt, SUPPORTED_CONVERTER_FORMATS[i]);
        foundFile = fs_file_exists(searchFileName );
        if(foundFile){
            foundIndex = i;
            break;
        }
    }

    ASSERT(foundFile);
    const char* ext = SUPPORTED_CONVERTER_FORMATS[foundIndex];
    const bool compileResult = convert_texture_dds(fileNameNoExt, ext, strcmp(ext, ".exr") ? TextureFormat::BC6H :TextureFormat::BC7);
    // This is quite a hack to get things working, as a concept.
    // I think I should instead add some .meta file or some authored material file where I can grab converter info from.
    return compileResult;
}

//======================================================================================================================
#endif //BEET_CONVERT_ON_DEMAND