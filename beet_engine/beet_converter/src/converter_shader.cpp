#include <beet_converter/converter_interface.h>
#include <beet_converter/converter_types.h>

#include <beet_shared/log.h>
#include <beet_shared/assert.h>
#include <beet_shared/filesystem.h>
#include <beet_shared/memory.h>
#include <beet_shared/c_string.h>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

//===INTERNAL_STRUCTS===================================================================================================
extern ConverterFileLocations g_converterDirs;

struct SpirVBinary {
    uint32_t *words;
    size_t size;
};
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static SpirVBinary compile_shader_to_SPIRV_vulkan_alloc(glslang_stage_t stage, const char *shaderSource, const char *fileName) {
    const glslang_input_t input = {
            .language = GLSLANG_SOURCE_GLSL,
            .stage = stage,
            .client = GLSLANG_CLIENT_VULKAN,
            .client_version = GLSLANG_TARGET_VULKAN_1_1,
            .target_language = GLSLANG_TARGET_SPV,
            .target_language_version = GLSLANG_TARGET_SPV_1_3,
            .code = shaderSource,
            .default_version = 450,
            .default_profile = GLSLANG_CORE_PROFILE,
            .force_default_version_and_profile = false,
            .forward_compatible = false,
            .messages = GLSLANG_MSG_DEFAULT_BIT,
            .resource = glslang_default_resource(),
    };

    glslang_shader_t *shader = glslang_shader_create(&input);

    SpirVBinary outBinary = {.words = nullptr, .size = 0,};
    if (!glslang_shader_preprocess(shader, &input)) {
        log_error(MSG_CONVERTER, "GLSL preprocessing failed %s\n", fileName);
        log_error(MSG_CONVERTER, "%s\n", glslang_shader_get_info_log(shader));
        log_error(MSG_CONVERTER, "%s\n", glslang_shader_get_info_debug_log(shader));
        log_error(MSG_CONVERTER, "%s\n", input.code);
        glslang_shader_delete(shader);
        return outBinary;
    }

    if (!glslang_shader_parse(shader, &input)) {
        log_error(MSG_CONVERTER, "GLSL parsing failed %s\n", fileName);
        log_error(MSG_CONVERTER, "%s\n", glslang_shader_get_info_log(shader));
        log_error(MSG_CONVERTER, "%s\n", glslang_shader_get_info_debug_log(shader));
        log_error(MSG_CONVERTER, "%s\n", glslang_shader_get_preprocessed_code(shader));
        glslang_shader_delete(shader);
        return outBinary;
    }

    glslang_program_t *program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
        log_error(MSG_CONVERTER, "GLSL linking failed %s\n", fileName);
        log_error(MSG_CONVERTER, "%s\n", glslang_program_get_info_log(program));
        log_error(MSG_CONVERTER, "%s\n", glslang_program_get_info_debug_log(program));
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        return outBinary;
    }

    glslang_program_SPIRV_generate(program, stage);

    outBinary.size = glslang_program_SPIRV_get_size(program);
    outBinary.words = (uint32_t *) mem_zalloc(outBinary.size * sizeof(uint32_t));
    glslang_program_SPIRV_get(program, outBinary.words);

    const char *spirv_messages = glslang_program_SPIRV_get_messages(program);
    if (spirv_messages) {
        log_error(MSG_CONVERTER, "(%s) %s\b", fileName, spirv_messages);
    }

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    return outBinary;
}

static glslang_stage_t glsl_stage_from_path(const char *inPath) {
    const char *fileExtension = c_str_search_reverse(inPath, ".");
    if (c_str_equal(fileExtension, ".vert")) {
        return GLSLANG_STAGE_VERTEX;
    }
    if (c_str_equal(fileExtension, ".frag")) {
        return GLSLANG_STAGE_FRAGMENT;
    }
    //TODO:CONVERTER Add more supported stages

    SANITY_CHECK();
    return GLSLANG_STAGE_COUNT; // invalid
}
//======================================================================================================================

//===API================================================================================================================
bool convert_shader_spv(const char *localAssetDir) {
    char inPath[256] = {};
    sprintf(inPath, "%s%s", g_converterDirs.rawAssetDir.c_str(), localAssetDir);
    char outPath[256] = {};
    sprintf(outPath, "%s%s", g_converterDirs.targetAssetDir.c_str(), localAssetDir);

    if (!fs_file_exists(inPath)) {
        return false;
    }

    if (!converter_cache_check_needs_convert(outPath, inPath)) {
        return true;
    }

    if (FILE *fp = fopen(inPath, "r")) {
        size_t fileSize = fs_file_size(inPath);
        char *shader = (char *) mem_zalloc(fileSize);
        fread(shader, fileSize, 1, fp);
        fclose(fp);
        fp = nullptr;

        const glslang_stage_t stage = glsl_stage_from_path(localAssetDir);
        glslang_initialize_process();
        SpirVBinary spvCode = {.words = nullptr, .size = 0};
        spvCode = compile_shader_to_SPIRV_vulkan_alloc(stage, shader, localAssetDir);
        glslang_finalize_process();
        mem_free(shader);
        if (spvCode.words) {
            fs_mkdir_recursive(outPath);
            fp = fopen(outPath, "wb");
            fwrite((char *) spvCode.words, spvCode.size * sizeof(*spvCode.words), 1, fp);
            fclose(fp);
            fp = nullptr;
            mem_free(spvCode.words);
            return true;
        }
    }
    return false;
}
//======================================================================================================================