#ifndef BEETROOT_CONVERTER_GLTF_PARSER_H
#define BEETROOT_CONVERTER_GLTF_PARSER_H

#include <beet_shared/feature_defines.h>

#define FEATURE_BEET_GLTF_PARSER FEATURE_OFF
#if CHECK_FEATURE(FEATURE_BEET_GLTF_PARSER)
//===PUBLIC_STRUCTS=====================================================================================================
//3.6.2.2. Accessor Data Types
enum GltfComponentTypesEnum {
    GLTF_COMPONENT_UNDEFINED = 0,
    GLTF_INT_8 = 5120,              // [ Bits:  8 ] [ Type: Signed byte ]
    GLTF_UINT_8 = 5121,             // [ Bits:  8 ] [ Type: Unsigned byte ]
    GLTF_INT_16 = 5122,             // [ Bits: 16 ] [ Type: Signed short ]
    GLTF_UINT_16 = 5123,            // [ Bits: 16 ] [ Type: Unsigned short ]
    GLTF_COMPONENT_UNUSED = 5124,   // [ Bits: ?? ] [ Type: Not defined in the spec likely Signed int ]
    GLTF_UINT_32 = 5125,            // [ Bits: 32 ] [ Type: Unsigned int ]
    GLTF_FLOAT_32 = 5126,           // [ Bits: 32 ] [ Type: Float ]
};

union GltfComponentType {
    int8_t int8;
    uint8_t uint8;
    int16_t int16;
    uint16_t uint16;
    uint32_t uint32;
    float float32;
};

enum GltfAccessorType : uint32_t {
    GLTF_ACCESSOR_UNDEFINED,
    GLTF_SCALAR,
    GLTF_VEC2,
    GLTF_VEC3,
    GLTF_VEC4,
    GLTF_MAT2,
    GLTF_MAT3,
    GLTF_MAT4,
};

struct GltfBufferViews {
    uint32_t bufferIndex = {};
    uint32_t byteOffset = {};
    uint32_t byteLength = {};
    uint32_t byteStride = {};
    uint32_t target = {};
    char name[GLTF_STR_BUFFER_VIEWS_NAME_SIZE] = {};
};

struct GltfTextures {
    uint32_t sampler = {};
    uint32_t source = {};
};

enum GltfWrapMode : uint32_t {
    GLTF_WRAP_MODE_UNDEFINED = 0,
    GLTF_WRAP_MODE_CLAMP_TO_EDGE = 33071,
    GLTF_WRAP_MODE_MIRRORED_REPEAT = 33648,
    GLTF_WRAP_MODE_REPEAT = 10497,
    //WebGL spec also has TEXTURE_WRAP_S & TEXTURE_WRAP_T
};

enum GltfMagnificationFilter : uint32_t {
    GLTF_SAMPLER_MAG_FILTER_UNDEFINED = 0,
    GLTF_SAMPLER_MAG_FILTER_NEAREST = 9728,
    GLTF_SAMPLER_MAG_FILTER_LINEAR = 9729,
};

enum GltfMinificationFilter : uint32_t {
    GLTF_SAMPLER_MIN_FILTER_UNDEFINED = 0,
    GLTF_SAMPLER_MIN_FILTER_NEAREST = 9728,
    GLTF_SAMPLER_MIN_FILTER_LINEAR = 9729,
    GLTF_SAMPLER_MIN_FILTER_NEAREST_MIPMAP_NEAREST = 9984,
    GLTF_SAMPLER_MIN_FILTER_LINEAR_MIPMAP_NEAREST = 9985,
    GLTF_SAMPLER_MIN_FILTER_NEAREST_MIPMAP_LINEAR = 9986,
    GLTF_SAMPLER_MIN_FILTER_LINEAR_MIPMAP_LINEAR = 9987,
};

struct GltfSamplers {
    GltfMagnificationFilter magFilter = {};
    GltfMinificationFilter minFilter = {};
    GltfWrapMode wrapS = {GLTF_WRAP_MODE_REPEAT};
    GltfWrapMode wrapT = {GLTF_WRAP_MODE_REPEAT};
};

constexpr size_t GLTF_STR_MEDIA_TYPE_SIZE = {128};
constexpr size_t GLTF_STR_MEDIA_NAME_SIZE = {256};
constexpr size_t GLTF_STR_URI_NAME_SIZE = {256};
struct GltfImages {
    uint32_t bufferView = {};
    char mediaType[GLTF_STR_MEDIA_TYPE_SIZE] = {}; //mimeType
    char name[GLTF_STR_MEDIA_NAME_SIZE] = {};
    char uri[GLTF_STR_URI_NAME_SIZE] = {}; //TODO we probably don't want to load textures during parse so a name of the texture is probably fine.
};

constexpr size_t GLTF_STR_ACCESSOR_NAME_SIZE = {128};
struct GltfAccessor {
    GltfComponentTypesEnum componentType = {GLTF_COMPONENT_UNDEFINED};
    GltfAccessorType accessorType = {GLTF_ACCESSOR_UNDEFINED};
    uint32_t bufferViewIndex = {};
    uint32_t byteOffset = {};
    uint32_t count = {};
    std::vector<GltfComponentType> max = {};
    std::vector<GltfComponentType> min = {};
    char name[GLTF_STR_ACCESSOR_NAME_SIZE] = {};
};

constexpr size_t GLTF_STR_GENERATOR_SIZE = {256};
constexpr size_t GLTF_STR_VERSION_SIZE = {32};
constexpr size_t GLTF_STR_COPYRIGHT_SIZE = {256};
struct GltfAsset {
    char generator[GLTF_STR_GENERATOR_SIZE] = {};
    char version[GLTF_STR_VERSION_SIZE] = {};
    char copyright[GLTF_STR_COPYRIGHT_SIZE] = {};
};

constexpr size_t GLTF_STR_SCENE_NAME_SIZE = {256};
struct GltfScene {
    char name[GLTF_STR_SCENE_NAME_SIZE] = {};
    std::vector<uint32_t> nodes = {};
};

constexpr uint32_t GLTF_INDEX_NOT_SET = {UINT32_MAX};

constexpr size_t GLTF_STR_NODE_NAME_SIZE = {256};
struct GltfNode {
    char name[GLTF_STR_NODE_NAME_SIZE] = {};
    uint32_t mesh = {GLTF_INDEX_NOT_SET};
    uint32_t camera = {GLTF_INDEX_NOT_SET};
    vec3f scale = {};
    vec3f translation = {};
    quat rotation = {};
};

//A.27. JSON Schema for Mesh Primitive
enum GltfTopologyMode : int32_t {
    GLTF_TOPOLOGY_MODE_POINTS = 0,
    GLTF_TOPOLOGY_MODE_LINES = 1,
    GLTF_TOPOLOGY_MODE_LINE_LOOP = 2,
    GLTF_TOPOLOGY_MODE_LINE_STRIP = 3,
    GLTF_TOPOLOGY_MODE_TRIANGLES = 4,
    GLTF_TOPOLOGY_MODE_TRIANGLE_STRIP = 5,
    GLTF_TOPOLOGY_MODE_TRIANGLE_FAN = 6,
};

struct GltfPrimitives {
    uint32_t attrib_normal = {GLTF_INDEX_NOT_SET};
    uint32_t attrib_position = {GLTF_INDEX_NOT_SET};
    uint32_t attrib_tangent = {GLTF_INDEX_NOT_SET};
    uint32_t attrib_tex_coord_0 = {GLTF_INDEX_NOT_SET};
    //uint32_t attrib_tex_coord_n = {GLTF_INDEX_NOT_SET};
    //uint32_t attrib_color_n = {GLTF_INDEX_NOT_SET};
    //uint32_t attrib_joints_n = {GLTF_INDEX_NOT_SET};
    //uint32_t attrib_weights_n = {GLTF_INDEX_NOT_SET};

    uint32_t indices = {GLTF_INDEX_NOT_SET};
    uint32_t material = {GLTF_INDEX_NOT_SET};
    GltfTopologyMode mode = {GLTF_TOPOLOGY_MODE_TRIANGLES};
};

constexpr size_t GLTF_STR_MESH_NAME_SIZE = {256};
struct GltfMesh {
    char name[GLTF_STR_MESH_NAME_SIZE] = {};
    std::vector<GltfPrimitives> primitives = {};
};

struct GltfTexture {
    uint32_t index = {GLTF_INDEX_NOT_SET};
    uint32_t texCoord = {GLTF_INDEX_NOT_SET};
};

constexpr size_t GLTF_STR_ALPHA_MODE_NAME_SIZE = {256};
enum GltfAlphaMode {
    GLTF_ALPHA_MODE_OPAQUE = 0,
    GLTF_ALPHA_MODE_MASK = 1,
    GLTF_ALPHA_MODE_BLEND = 2,
};

constexpr size_t GLTF_STR_BUFFER_VIEWS_NAME_SIZE = {128};

constexpr size_t GLTF_STR_MATERIAL_NAME_SIZE = {256};
struct GltfMaterial {
    bool doubleSided = {false};
    char name[GLTF_STR_MESH_NAME_SIZE] = {};
    struct {
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        vec4f baseColorFactor = {};
        GltfTexture baseColorTexture = {};
        GltfTexture metallicRoughnessTexture = {};
    } pbrMetallicRoughness;
    GltfTexture normalTexture = {};
    GltfTexture occlusionTexture = {};
    GltfTexture emissiveTexture = {};
    vec3f emissiveFactor = {};
    GltfAlphaMode alphaMode = {};
    float alphaCutoff = {};
};

struct GltfBuffer {
    size_t byteLength = {};
    std::vector<char> binaryData = {};
};

constexpr size_t GLTF_STR_EXTENSION_NAME_SIZE = {128};
struct GltfUsedExtension {
    char itemName[GLTF_STR_EXTENSION_NAME_SIZE] = {};
};

enum GLTF_CAMERA_TYPE {
    GLTF_CAMERA_TYPE_PERSPECTIVE,
    GLTF_CAMERA_TYPE_ORTHOGRAPHIC,
};

constexpr size_t GLTF_STR_CAMERA_NAME_SIZE = {128};
struct GltfCamera {
    GLTF_CAMERA_TYPE cameraType = {};
    float zFar = {1000.0f};
    float zNear = {0.0f};
    char name[GLTF_STR_CAMERA_NAME_SIZE] = {};
    struct {
        float aspectRatio = {1.0f};
        float yFov = {0.9f}; // radians
    } perspective;
    struct {
        float xMag = {1.0f};
        float yMag = {1.0f};
    } orthographic;
};

constexpr size_t GLTF_STR_PATH_SIZE = {256};
struct GltfIntermediate {
    char path[GLTF_STR_PATH_SIZE] = {};
    GltfAsset asset = {};
    uint32_t sceneCount = {};
    std::vector<GltfScene> scenes = {};
    std::vector<GltfNode> nodes = {};
    std::vector<GltfMesh> meshes = {};
    std::vector<GltfMaterial> materials = {};
    std::vector<GltfAccessor> accessors = {};
    std::vector<GltfBufferViews> bufferViews = {};
    std::vector<GltfTextures> textures = {};
    std::vector<GltfSamplers> samplers = {};
    std::vector<GltfImages> images = {};
    std::vector<GltfBuffer> buffers = {};
    std::vector<GltfCamera> cameras = {};
    std::vector<GltfUsedExtension> usedExtensions = {};
}
//======================================================================================================================

//===API================================================================================================================
size_t gltf_component_type_size_lookup(GltfComponentTypesEnum type);
const char *gltf_component_type_lookup(GltfComponentTypesEnum type);
const char *gltf_accessor_type_lookup(GltfAccessorType type);
uint32_t gltf_accessor_type_size_lookup(GltfAccessorType type);
GltfAccessorType gltf_accessor_type_lookup(const char *str);
GltfAlphaMode gltf_alpha_mode_type_lookup(const char *alphaModeStr);

bool parse_gltf_get_binary_data(std::vector<char> &outData, const char *uri, const char *gltfPath);
void parse_gltf_buffers(std::vector<GltfBuffer> &outBuffer, const rapidjson::Value &bufferValue);
void parse_gltf_texture(GltfTexture &outTexture, const rapidjson::Value &textureValue);
void parse_gltf_materials(std::vector<GltfMaterial> &outMaterials, const rapidjson::Value &materialValue);
void parse_gltf_meshes(std::vector<GltfMesh> &outMeshes, const rapidjson::Value &meshValue);
void parse_gltf_nodes(std::vector<GltfNode> &outNodes, const rapidjson::Value &nodeValue);
void parse_gltf_scenes(std::vector<GltfScene> &outScenes, const rapidjson::Value &sceneValue);

void parse_gltf_samplers(std::vector<GltfSamplers> &outSamplers, const rapidjson::Value &samplerValue);
void parse_gltf_images(std::vector<GltfImages> &outImages, const rapidjson::Value &imageValue);
void parse_gltf_textures(std::vector<GltfTextures> &outTextures, const rapidjson::Value &textureValue);
void parse_gltf_buffer_views(std::vector<GltfBufferViews> &outBufferViews, const rapidjson::Value &bufferViewValue);
void parse_gltf_cameras(std::vector<GltfCamera> &outCameras, const rapidjson::Value &cameraValue);
void parse_gltf_extensions_used(std::vector<GltfUsedExtension> &outUsedExtensions, const rapidjson::Value &usedExtensionsValue);
void parse_gltf_accessors(std::vector<GltfAccessor> &outAccessors, const rapidjson::Value &accessorsValue);
void parse_gltf_asset(GltfAsset &outAsset, const rapidjson::Value &jsonValue);

void gltf_dump_intermediate(const GltfIntermediate &gltfData);

void *gltf_pull_out_binary_data_alloc(const uint32_t inAccessorIndex, size_t &outCount, GltfAccessorType &outAccessor, GltfComponentTypesEnum &outType);
void gltf_parse_json(const char *gltfPath, const char *targetWriteDir, AssetPackage &outPackage);
//======================================================================================================================
#endif //CHECK_FEATURE(FEATURE_BEET_GLTF_PARSER)
#endif //BEETROOT_CONVERTER_GLTF_PARSER_H
