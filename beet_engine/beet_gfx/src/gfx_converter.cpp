#include <beet_gfx/gfx_converter.h>
#include <beet_converter/converter_interface.h>
#include <beet_shared/assert.h>
#include <beet_shared/log.h>

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
    log_info(MSG_GFX, "Option force recompile : [%s]\n", optionForceReconvert ? "TRUE" : "FALSE");
}

bool gfx_convert_shader_spv(const char *localAssetPath) {
    const bool compileResult = convert_shader_spv(localAssetPath);
    ASSERT_MSG(compileResult, "Err: gfx failed to compile shader %s \n", localAssetPath);
    return compileResult;
}
//======================================================================================================================
#endif //BEET_CONVERT_ON_DEMAND