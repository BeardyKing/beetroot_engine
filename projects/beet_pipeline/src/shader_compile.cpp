#include <beet_pipeline/shader_compile.h>
#include <beet_pipeline/pipeline_defines.h>
#include <beet_pipeline/pipeline_cache.h>

#include <beet_shared/log.h>
#include <beet_shared/filesystem.h>

#include <fmt/format.h>

//===API================================================================================================================
void pipeline_build_shader_spv(const std::string &readPath, const std::string &writePath) {
    const std::string inPath = fmt::format("{}{}", PIPELINE_SHADER_DIR, readPath);
    const std::string outPath = fmt::format("{}{}", CLIENT_RUNTIME_SHADER_DIR, writePath);
    const std::string cmd = fmt::format("{} {} {} {}", GLSL_VALIDATOR_EXE_PATH, "-V -o", outPath, inPath);

    if (pipeline_cache_should_convert(outPath, inPath)) {
        log_info(MSG_PIPELINE, "shader: %s \n", outPath.c_str());
        system(cmd.c_str());
    }
}

void pipeline_shader_log() {
    log_info(MSG_PIPELINE, "\n")
    log_info(MSG_PIPELINE, "===========================\n")
    log_info(MSG_PIPELINE, "===BUILDING SHADERS========\n")
    log_info(MSG_PIPELINE, "===========================\n")
    log_info(MSG_PIPELINE, "\n")
}
//======================================================================================================================
