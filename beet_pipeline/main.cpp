#include <beet_shared/log.h>
#include <ispc_texcomp.h>

#define CGLTF_IMPLEMENTATION

#include <cgltf.h>

#include <beet_pipeline/pipeline_commandlines.h>
#include <beet_shared/assert.h>
#include <beet_pipeline/shader_compile.h>
#include "beet_pipeline/pipeline_defines.h"
#include "beet_shared/filesystem.h"

// investigate https://github.com/floooh/sokol-samples/blob/master/sapp/cgltf-sapp.c
// gltf_parse_images() / gltf_parse()
constexpr uint32_t DBG_MAX_SIZE = 32;
struct Package {
    int num_buffers;
    int num_images;
    int num_pipelines;
    int num_materials;
    int num_primitives; // aka 'submeshes'
    int num_meshes;
    int num_nodes;
    char buffers[DBG_MAX_SIZE];
//    ImageSampler image_samplers[DBG_MAX_SIZE];
//    Pipeline pipelines[DBG_MAX_SIZE];
//    MaterialProperties materials[DBG_MAX_SIZE];
//    RawPrimitives primitives[DBG_MAX_SIZE];
//    RawMesh meshes[DBG_MAX_SIZE];
//    NodeHierarchy nodes[DBG_MAX_SIZE];
};

void gltf_parsing_tests() {
    log_info(MSG_PIPELINE, "hello pipeline\n");
    const char *fileName = BEET_CMAKE_PIPELINE_ASSETS_DIR "scenes/example_scene.gltf";

    cgltf_options options = {};
    cgltf_data *data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, fileName, &data);
    ASSERT_MSG(result == cgltf_result_success, "Err: failed to parse file: %s \n", fileName);
    if (result == cgltf_result_success) {
        log_info(MSG_PIPELINE, "parsed file successfully\n");
    }
}

void build_spv_from_source() {
    fs_mkdir_recursive(CLIENT_RUNTIME_SHADER_DIR);
    pipeline_shader_log();
    {
        pipeline_build_shader_spv("indirectdraw/indirectdraw.frag", "indirectdraw.frag.spv");
        pipeline_build_shader_spv("indirectdraw/indirectdraw.vert", "indirectdraw.vert.spv");

        pipeline_build_shader_spv("lit/lit.frag", "lit.frag.spv");
        pipeline_build_shader_spv("lit/lit.vert", "lit.vert.spv");

        pipeline_build_shader_spv("sky/sky.frag", "sky.frag.spv");
        pipeline_build_shader_spv("sky/sky.vert", "sky.vert.spv");
    }
}

int main(int argc, char **argv) {
    commandline_init(argc, argv);
    if (commandline_get_arg(CLArgs::help).enabled) {
        commandline_show_commands();
        return 0;
    }

    build_spv_from_source();
}