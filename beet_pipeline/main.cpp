#include <beet_shared/log.h>
#include <ispc_texcomp.h>

#define CGLTF_IMPLEMENTATION

#include <cgltf.h>

#include <beet_pipeline/pipeline_commandlines.h>
#include <beet_shared/assert.h>
#include <beet_pipeline/shader_compile.h>
#include <beet_converter/converter_interface.h>

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
    {
        ASSERT(convert_shader_spv("shaders/indirectdraw/indirectdraw.frag"));
        ASSERT(convert_shader_spv("shaders/indirectdraw/indirectdraw.vert"));

        ASSERT(convert_shader_spv("shaders/lit/lit.frag"));
        ASSERT(convert_shader_spv("shaders/lit/lit.vert"));

        ASSERT(convert_shader_spv("shaders/sky/sky.frag"));
        ASSERT(convert_shader_spv("shaders/sky/sky.vert"));
    }
}

int main(int argc, char **argv) {
    commandline_init(argc, argv);
    if (commandline_get_arg(CLArgs::help).enabled) {
        commandline_show_commands();
        return 0;
    }
    converter_init(BEET_CMAKE_PIPELINE_ASSETS_DIR, BEET_CMAKE_RUNTIME_ASSETS_DIR);
    converter_option_set_ignore_cache(commandline_get_arg(CLArgs::ignoreConvertCache).enabled);

    build_spv_from_source();
}