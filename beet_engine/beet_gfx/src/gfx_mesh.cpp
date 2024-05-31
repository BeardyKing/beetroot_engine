#include <beet_gfx/gfx_mesh.h>
#include <beet_shared/assert.h>
#include <beet_gfx/gfx_buffer.h>
#include <beet_gfx/gfx_command.h>
#include <beet_gfx/gfx_types.h>

//===INTERNAL_STRUCTS===================================================================================================
extern VulkanBackend g_vulkanBackend;
//======================================================================================================================

//===API================================================================================================================
void gfx_mesh_create_immediate(const RawMesh &rawMesh, GfxMesh &outMesh) {
    ASSERT((rawMesh.vertexCount > 0) && (rawMesh.indexCount > 0))

    struct StagingBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };
    StagingBuffer vertexStaging = {};
    StagingBuffer indexStaging = {};

    const size_t vertexBufferSize = sizeof(GfxVertex) * rawMesh.vertexCount;
    const size_t indexBufferSize = sizeof(uint32_t) * rawMesh.indexCount;

    outMesh.indexCount = rawMesh.indexCount;
    outMesh.vertCount = rawMesh.vertexCount;

    // Create staging buffers
    VkResult vertexCreateStageRes = gfx_buffer_create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBufferSize,
            vertexStaging.buffer,
            vertexStaging.memory,
            rawMesh.vertexData
    );
    ASSERT(vertexCreateStageRes == VK_SUCCESS)

    VkResult indexCreateStageRes = gfx_buffer_create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            indexBufferSize,
            indexStaging.buffer,
            indexStaging.memory,
            rawMesh.indexData
    );
    ASSERT(indexCreateStageRes == VK_SUCCESS)

    // Create device local buffers
    const VkResult vertexCreateDeviceLocalRes = gfx_buffer_create(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBufferSize,
            outMesh.vertBuffer,
            outMesh.vertMemory,
            nullptr
    );
    ASSERT(vertexCreateDeviceLocalRes == VK_SUCCESS)
    // Index buffer
    const VkResult indexCreateDeviceLocalRes = gfx_buffer_create(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indexBufferSize,
            outMesh.indexBuffer,
            outMesh.indexMemory,
            nullptr
    );
    ASSERT(indexCreateDeviceLocalRes == VK_SUCCESS)

    gfx_command_begin_immediate_recording();
    {
        VkBufferCopy copyRegion = {};
        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(g_vulkanBackend.immediateCommandBuffer, vertexStaging.buffer, outMesh.vertBuffer, 1, &copyRegion);

        copyRegion.size = indexBufferSize;
        vkCmdCopyBuffer(g_vulkanBackend.immediateCommandBuffer, indexStaging.buffer, outMesh.indexBuffer, 1, &copyRegion);
    }
    gfx_command_end_immediate_recording();

    vkDestroyBuffer(g_vulkanBackend.device, vertexStaging.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, vertexStaging.memory, nullptr);
    vkDestroyBuffer(g_vulkanBackend.device, indexStaging.buffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, indexStaging.memory, nullptr);
}


void gfx_mesh_create_octahedron_immediate(GfxMesh &outMesh) {
    const uint32_t vertexCount = 24;
    GfxVertex vertexData[vertexCount] = {
            //===POS================//===NORMAL===========================//===UV=======//===COLOUR========
            {{+0.0f, -0.0f, +1.0f}, {+0.577350f, -0.577350f, +0.577350f}, {0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, -1.0f, -0.0f}, {+0.577350f, -0.577350f, +0.577350f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+1.0f, +0.0f, +0.0f}, {+0.577350f, -0.577350f, +0.577350f}, {0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+1.0f, +0.0f, +0.0f}, {+0.577350f, -0.577350f, -0.577350f}, {0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, -1.0f, -0.0f}, {+0.577350f, -0.577350f, -0.577350f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, +0.0f, -1.0f}, {+0.577350f, -0.577350f, -0.577350f}, {0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, +0.0f, -1.0f}, {-0.577350f, -0.577350f, -0.577350f}, {0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, -1.0f, -0.0f}, {-0.577350f, -0.577350f, -0.577350f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-1.0f, +0.0f, +0.0f}, {-0.577350f, -0.577350f, -0.577350f}, {1.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{-1.0f, +0.0f, +0.0f}, {-0.577350f, -0.577350f, +0.577350f}, {1.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, -1.0f, -0.0f}, {-0.577350f, -0.577350f, +0.577350f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, -0.0f, +1.0f}, {-0.577350f, -0.577350f, +0.577350f}, {0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, +0.0f, -1.0f}, {+0.577350f, +0.577350f, -0.577350f}, {0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, +1.0f, +0.0f}, {+0.577350f, +0.577350f, -0.577350f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+1.0f, +0.0f, +0.0f}, {+0.577350f, +0.577350f, -0.577350f}, {0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{-1.0f, +0.0f, +0.0f}, {-0.577350f, +0.577350f, -0.577350f}, {1.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, +1.0f, +0.0f}, {-0.577350f, +0.577350f, -0.577350f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, +0.0f, -1.0f}, {-0.577350f, +0.577350f, -0.577350f}, {0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, -0.0f, +1.0f}, {-0.577350f, +0.577350f, +0.577350f}, {0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, +1.0f, +0.0f}, {-0.577350f, +0.577350f, +0.577350f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{-1.0f, +0.0f, +0.0f}, {-0.577350f, +0.577350f, +0.577350f}, {1.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+1.0f, +0.0f, +0.0f}, {+0.577350f, +0.577350f, +0.577350f}, {0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, +1.0f, +0.0f}, {+0.577350f, +0.577350f, +0.577350f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f, -0.0f, +1.0f}, {+0.577350f, +0.577350f, +0.577350f}, {0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    };

    const uint32_t indexCount = 24;
    uint32_t indexData[indexCount] = {
            0, 1, 2,
            3, 4, 5,
            6, 7, 8,
            9, 10, 11,
            12, 13, 14,
            15, 16, 17,
            18, 19, 20,
            21, 22, 23,
    };

    const RawMesh rawMesh = {
            vertexData,
            indexData,
            vertexCount,
            indexCount,
    };

    gfx_mesh_create_immediate(rawMesh, outMesh);
}

#if IN_DEV_RUNTIME_GLTF_LOADING

#include "../../../beet_pipeline/third/rapidjson/include/rapidjson/document.h"
#include <beet_shared/c_string.h>
#include <beet_shared/log.h>

const char gltfJson[] = "{\n"
                        "    \"asset\" : {\n"
                        "        \"generator\" : \"Khronos glTF Blender I/O v3.3.27\",\n"
                        "        \"version\" : \"2.0\"\n"
                        "    },\n"
                        "    \"scene\" : 0,\n"
                        "    \"scenes\" : [\n"
                        "        {\n"
                        "            \"name\" : \"Scene\",\n"
                        "            \"nodes\" : [\n"
                        "                0\n"
                        "            ]\n"
                        "        }\n"
                        "    ],\n"
                        "    \"nodes\" : [\n"
                        "        {\n"
                        "            \"mesh\" : 0,\n"
                        "            \"name\" : \"Cube\"\n"
                        "        }\n"
                        "    ],\n"
                        "    \"materials\" : [\n"
                        "        {\n"
                        "            \"doubleSided\" : true,\n"
                        "            \"name\" : \"Material\",\n"
                        "            \"pbrMetallicRoughness\" : {\n"
                        "                \"baseColorTexture\" : {\n"
                        "                    \"index\" : 0\n"
                        "                },\n"
                        "                \"metallicFactor\" : 0,\n"
                        "                \"roughnessFactor\" : 0.5\n"
                        "            }\n"
                        "        }\n"
                        "    ],\n"
                        "    \"meshes\" : [\n"
                        "        {\n"
                        "            \"name\" : \"Cube\",\n"
                        "            \"primitives\" : [\n"
                        "                {\n"
                        "                    \"attributes\" : {\n"
                        "                        \"POSITION\" : 0,\n"
                        "                        \"NORMAL\" : 1,\n"
                        "                        \"TEXCOORD_0\" : 2\n"
                        "                    },\n"
                        "                    \"indices\" : 3,\n"
                        "                    \"material\" : 0\n"
                        "                }\n"
                        "            ]\n"
                        "        }\n"
                        "    ],\n"
                        "    \"textures\" : [\n"
                        "        {\n"
                        "            \"sampler\" : 0,\n"
                        "            \"source\" : 0\n"
                        "        }\n"
                        "    ],\n"
                        "    \"images\" : [\n"
                        "        {\n"
                        "            \"bufferView\" : 4,\n"
                        "            \"mimeType\" : \"image/png\",\n"
                        "            \"name\" : \"hi_16x16\"\n"
                        "        }\n"
                        "    ],\n"
                        "    \"accessors\" : [\n"
                        "        {\n"
                        "            \"bufferView\" : 0,\n"
                        "            \"componentType\" : 5126,\n"
                        "            \"count\" : 24,\n"
                        "            \"max\" : [\n"
                        "                1,\n"
                        "                1,\n"
                        "                1\n"
                        "            ],\n"
                        "            \"min\" : [\n"
                        "                -1,\n"
                        "                -1,\n"
                        "                -1\n"
                        "            ],\n"
                        "            \"type\" : \"VEC3\"\n"
                        "        },\n"
                        "        {\n"
                        "            \"bufferView\" : 1,\n"
                        "            \"componentType\" : 5126,\n"
                        "            \"count\" : 24,\n"
                        "            \"type\" : \"VEC3\"\n"
                        "        },\n"
                        "        {\n"
                        "            \"bufferView\" : 2,\n"
                        "            \"componentType\" : 5126,\n"
                        "            \"count\" : 24,\n"
                        "            \"type\" : \"VEC2\"\n"
                        "        },\n"
                        "        {\n"
                        "            \"bufferView\" : 3,\n"
                        "            \"componentType\" : 5123,\n"
                        "            \"count\" : 36,\n"
                        "            \"type\" : \"SCALAR\"\n"
                        "        }\n"
                        "    ],\n"
                        "    \"bufferViews\" : [\n"
                        "        {\n"
                        "            \"buffer\" : 0,\n"
                        "            \"byteLength\" : 288,\n"
                        "            \"byteOffset\" : 0,\n"
                        "            \"target\" : 34962\n"
                        "        },\n"
                        "        {\n"
                        "            \"buffer\" : 0,\n"
                        "            \"byteLength\" : 288,\n"
                        "            \"byteOffset\" : 288,\n"
                        "            \"target\" : 34962\n"
                        "        },\n"
                        "        {\n"
                        "            \"buffer\" : 0,\n"
                        "            \"byteLength\" : 192,\n"
                        "            \"byteOffset\" : 576,\n"
                        "            \"target\" : 34962\n"
                        "        },\n"
                        "        {\n"
                        "            \"buffer\" : 0,\n"
                        "            \"byteLength\" : 72,\n"
                        "            \"byteOffset\" : 768,\n"
                        "            \"target\" : 34963\n"
                        "        },\n"
                        "        {\n"
                        "            \"buffer\" : 0,\n"
                        "            \"byteLength\" : 430,\n"
                        "            \"byteOffset\" : 840\n"
                        "        }\n"
                        "    ],\n"
                        "    \"samplers\" : [\n"
                        "        {\n"
                        "            \"magFilter\" : 9729,\n"
                        "            \"minFilter\" : 9987\n"
                        "        }\n"
                        "    ],\n"
                        "    \"buffers\" : [\n"
                        "        {\n"
                        "            \"byteLength\" : 1272,\n"
                        "            \"uri\" : \"data:application/octet-stream;base64,AACAPwAAgD8AAIC/AACAPwAAgD8AAIC/AACAPwAAgD8AAIC/AACAPwAAgL8AAIC/AACAPwAAgL8AAIC/AACAPwAAgL8AAIC/AACAPwAAgD8AAIA/AACAPwAAgD8AAIA/AACAPwAAgD8AAIA/AACAPwAAgL8AAIA/AACAPwAAgL8AAIA/AACAPwAAgL8AAIA/AACAvwAAgD8AAIC/AACAvwAAgD8AAIC/AACAvwAAgD8AAIC/AACAvwAAgL8AAIC/AACAvwAAgL8AAIC/AACAvwAAgL8AAIC/AACAvwAAgD8AAIA/AACAvwAAgD8AAIA/AACAvwAAgD8AAIA/AACAvwAAgL8AAIA/AACAvwAAgL8AAIA/AACAvwAAgL8AAIA/AAAAAAAAAAAAAIC/AAAAAAAAgD8AAACAAACAPwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIC/AACAPwAAAAAAAACAAAAAAAAAAAAAAIA/AAAAAAAAgD8AAACAAACAPwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIA/AACAPwAAAAAAAACAAACAvwAAAAAAAACAAAAAAAAAAAAAAIC/AAAAAAAAgD8AAACAAACAvwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIC/AACAvwAAAAAAAACAAAAAAAAAAAAAAIA/AAAAAAAAgD8AAACAAACAvwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIA/AACAPwAAAAAAAIA/AAAAAAAAgD8AAAAAAACAPwAAAAAAAIA/AACAPwAAgD8AAIA/AACAPwAAAAAAAIA/AACAPwAAAAAAAAAAAACAPwAAgD8AAIA/AACAPwAAAAAAAIA/AACAPwAAAAAAAAAAAAAAAAAAAAAAAAAAAACAPwAAgD8AAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAIA/AAAAAAAAgD8AAAAAAACAPwAAAAAAAIA/AQAOABQAAQAUAAcACgAGABMACgATABcAFQASAAwAFQAMAA8AEAADAAkAEAAJABYABQACAAgABQAIAAsAEQANAAAAEQAAAAQAiVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAIAAACQkWg2AAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAFDSURBVDhPzZKxioNQEEVdIRoIiFj4A0IQBNtgOlsbm/gVlinyLSntxVr8A1tboySBFEGUICKISdyr7xGSane7PcU4M+9e5434NQwD8xdY+vw1PxvKsrzf77QAuBJ4PB51XZP8nfP5rGma53m0HgY6IQxDwzBOpxMpX4iiaFkWPLQGxLfdbnGWpikp38FkzKfFa0JRFIvFguO4PM+Px2PXdaSPK5mmGUURKQE1VFUF9W63W6/Xq9XKdV2yKC55OBwulwuRAWrAcdM0vu+rqqooCraM4xh9NBFlWZ5UI9TQ9/3tdlsul0EQ7Pd7nueTJEH/+XwizufzSTVCDdgGm202G0mSyIosOx5hGbyrbVsiA9TgOA7m2raNHAZBEHRdRz6bzVAiTqqJ6VsNGI3Ph4gciuv1SuZgtyzLECfVyH/4+T5gmG9Y7PkNFDxPhAAAAABJRU5ErkJgggAA\"\n"
                        "        }\n"
                        "    ]\n"
                        "}";

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

[[maybe_unused]] size_t GltfComponentTypeSizeLookup(GltfComponentTypesEnum type) {
    switch (type) {
        case GLTF_COMPONENT_UNDEFINED:
            return 0;
        case GLTF_INT_8:
            return sizeof(int8_t);
        case GLTF_UINT_8:
            return sizeof(uint8_t);
        case GLTF_INT_16:
            return sizeof(int16_t);
        case GLTF_UINT_16:
            return sizeof(uint16_t);
        case GLTF_COMPONENT_UNUSED:
            return 0; // likely should be sizeof(int32_t) but int32 is not defined in the spec
        case GLTF_UINT_32:
            return sizeof(uint32_t);
        case GLTF_FLOAT_32:
            return sizeof(float);
    }
    SANITY_CHECK()
    return 0;
}

const char *GltfComponentTypeLookup(GltfComponentTypesEnum type) {
    switch (type) {
        case GLTF_COMPONENT_UNDEFINED:
            return "GLTF_COMPONENT_UNDEFINED";
        case GLTF_INT_8:
            return "GLTF_INT_8";
        case GLTF_UINT_8:
            return "GLTF_UINT_8";
        case GLTF_INT_16:
            return "GLTF_INT_16";
        case GLTF_UINT_16:
            return "GLTF_UINT_16";
        case GLTF_COMPONENT_UNUSED:
            return "GLTF_COMPONENT_UNUSED";
        case GLTF_UINT_32:
            return "GLTF_UINT_32";
        case GLTF_FLOAT_32:
            return "GLTF_FLOAT_32";
    }
    SANITY_CHECK()
    return "";
}

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

const char *GltfAccessorTypeLookup(GltfAccessorType type) {
    switch (type) {
        case GLTF_ACCESSOR_UNDEFINED:
            return "GLTF_ACCESSOR_UNDEFINED";
        case GLTF_SCALAR:
            return "GLTF_SCALAR";
        case GLTF_VEC2:
            return "GLTF_VEC2";
        case GLTF_VEC3:
            return "GLTF_VEC3";
        case GLTF_VEC4:
            return "GLTF_VEC4";
        case GLTF_MAT2:
            return "GLTF_MAT2";
        case GLTF_MAT3:
            return "GLTF_MAT3";
        case GLTF_MAT4:
            return "GLTF_MAT4";
    }
    SANITY_CHECK()
    return "";
}

uint32_t GltfAccessorTypeSizeLookup(GltfAccessorType type) {
    switch (type) {
        case GLTF_ACCESSOR_UNDEFINED:
            return 0;
        case GLTF_SCALAR:
            return 1;
        case GLTF_VEC2:
            return 2;
        case GLTF_VEC3:
            return 3;
        case GLTF_VEC4:
        case GLTF_MAT2:
            return 4;
        case GLTF_MAT3:
            return 9;
        case GLTF_MAT4:
            return 16;
    }
    SANITY_CHECK()
    return 0;
}

GltfAccessorType AccessorTypeLookup(const char *str) {
    if (c_str_equal(str, "SCALAR")) {
        return GLTF_SCALAR;
    }
    if (c_str_equal(str, "VEC2")) {
        return GLTF_VEC2;
    }
    if (c_str_equal(str, "VEC3")) {
        return GLTF_VEC3;
    }
    if (c_str_equal(str, "VEC4")) {
        return GLTF_VEC4;
    }
    if (c_str_equal(str, "MAT2")) {
        return GLTF_MAT2;
    }
    if (c_str_equal(str, "MAT3")) {
        return GLTF_MAT3;
    }
    if (c_str_equal(str, "MAT4")) {
        return GLTF_MAT4;
    }
    SANITY_CHECK()
    return GLTF_ACCESSOR_UNDEFINED;
}

struct GltfBufferViews {
    uint32_t buffer = {};
    uint32_t byteOffset = {};
    uint32_t byteLength = {};
    uint32_t target = {};
};

struct GltfTextures {
    uint32_t sampler = {};
    uint32_t source = {};
};

struct GltfSamplers {
    uint32_t magFilter = {};
    uint32_t minFilter = {};
};

constexpr size_t GLTF_STR_MEDIA_TYPE_SIZE = {128};
constexpr size_t GLTF_STR_MEDIA_NAME_SIZE = {256};
struct GltfImages {
    uint32_t bufferView = {};
    char mediaType[GLTF_STR_MEDIA_TYPE_SIZE] = {}; //mimeType
    char name[GLTF_STR_MEDIA_NAME_SIZE] = {};
};

struct GltfAccessor {
    GltfComponentTypesEnum componentType = {GLTF_COMPONENT_UNDEFINED};
    GltfAccessorType accessorType = {GLTF_ACCESSOR_UNDEFINED};
    uint32_t bufferView = {};
    uint32_t byteOffset = {};
    uint32_t count = {};
    std::vector<GltfComponentType> max = {};
    std::vector<GltfComponentType> min = {};
};

constexpr size_t GLTF_STR_GENERATOR_SIZE = {256};
constexpr size_t GLTF_STR_VERSION_SIZE = {32};
struct GltfAsset {
    char generator[GLTF_STR_GENERATOR_SIZE] = {};
    char version[GLTF_STR_VERSION_SIZE] = {};
};

struct GltfIntermediate {
    GltfAsset asset = {};

    std::vector<GltfAccessor> accessors = {};
    std::vector<GltfBufferViews> bufferViews = {};
    std::vector<GltfTextures> textures = {};
    std::vector<GltfSamplers> samplers = {};
    std::vector<GltfImages> images = {};
} g_gltfData;

void parse_gltf_samplers(std::vector<GltfSamplers> &outSamplers, const rapidjson::Value &samplerValue) {
    ASSERT(samplerValue.IsArray())
    outSamplers.reserve(samplerValue.GetArray().Size());
    for (auto const &samplers: samplerValue.GetArray()) {
        GltfSamplers &gltfSamplers = outSamplers.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = samplers.MemberBegin(); itr != samplers.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "magFilter")) {
                ASSERT(itrValue.IsUint())
                gltfSamplers.magFilter = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "minFilter")) {
                ASSERT(itrValue.IsUint())
                gltfSamplers.minFilter = itrValue.GetUint();
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_images(std::vector<GltfImages> &outImages, const rapidjson::Value &imageValue) {
    ASSERT(imageValue.IsArray())
    outImages.reserve(imageValue.GetArray().Size());
    for (auto const &texture: imageValue.GetArray()) {
        GltfImages &gltfImages = outImages.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = texture.MemberBegin(); itr != texture.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "bufferView")) {
                ASSERT(itrValue.IsUint())
                gltfImages.bufferView = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "mimeType")) {
                ASSERT(itr->value.IsString())
                ASSERT(itr->value.GetStringLength() < GLTF_STR_MEDIA_TYPE_SIZE)
                sprintf(gltfImages.mediaType, "%s", itr->value.GetString());
                continue;
            } else if (c_str_equal(jsonString, "name")) {
                ASSERT(itr->value.IsString())
                ASSERT(itr->value.GetStringLength() < GLTF_STR_MEDIA_NAME_SIZE)
                sprintf(gltfImages.name, "%s", itr->value.GetString());
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_textures(std::vector<GltfTextures> &outTextures, const rapidjson::Value &textureValue) {
    ASSERT(textureValue.IsArray())
    outTextures.reserve(textureValue.GetArray().Size());
    for (auto const &texture: textureValue.GetArray()) {
        GltfTextures &gltfTextures = outTextures.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = texture.MemberBegin(); itr != texture.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "sampler")) {
                ASSERT(itrValue.IsUint())
                gltfTextures.sampler = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "source")) {
                ASSERT(itrValue.IsUint())
                gltfTextures.source = itrValue.GetUint();
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_buffer_views(std::vector<GltfBufferViews> &outBufferViews, const rapidjson::Value &bufferViewValue) {
    ASSERT(bufferViewValue.IsArray())
    outBufferViews.reserve(bufferViewValue.GetArray().Size());
    for (auto const &bufferView: bufferViewValue.GetArray()) {
        GltfBufferViews &gltfBufferViews = outBufferViews.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = bufferView.MemberBegin(); itr != bufferView.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "buffer")) {
                ASSERT(itrValue.IsUint())
                gltfBufferViews.buffer = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "byteOffset")) {
                ASSERT(itrValue.IsUint())
                gltfBufferViews.byteOffset = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "byteLength")) {
                ASSERT(itrValue.IsUint())
                gltfBufferViews.byteLength = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "target")) {
                ASSERT(itrValue.IsUint())
                gltfBufferViews.target = itrValue.GetUint();
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_accessors(std::vector<GltfAccessor> &outAccessors, const rapidjson::Value &accessorsValue) {
    ASSERT(accessorsValue.IsArray())
    outAccessors.reserve(accessorsValue.GetArray().Size());
    for (auto const &accessor: accessorsValue.GetArray()) {
        GltfAccessor &gltfAccessor = outAccessors.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = accessor.MemberBegin(); itr != accessor.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "componentType")) {
                ASSERT(itrValue.IsUint())
                gltfAccessor.componentType = GltfComponentTypesEnum(itrValue.GetUint());
                continue;
            } else if (c_str_equal(jsonString, "type")) {
                ASSERT(itrValue.IsString())
                gltfAccessor.accessorType = AccessorTypeLookup(itrValue.GetString());
                continue;
            } else if (c_str_equal(jsonString, "bufferView")) {
                ASSERT(itrValue.IsUint())
                gltfAccessor.bufferView = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "byteOffset")) {
                ASSERT(itrValue.IsUint())
                gltfAccessor.byteOffset = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "count")) {
                ASSERT(itrValue.IsUint())
                gltfAccessor.count = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "min") || c_str_equal(jsonString, "max")) {
                ASSERT(itrValue.IsArray())
                for (auto const &minMaxEle: itrValue.GetArray()) {
                    GltfComponentType minMaxVal = {};
                    switch (gltfAccessor.componentType) {
                        case GLTF_COMPONENT_UNDEFINED:
                        case GLTF_COMPONENT_UNUSED: SANITY_CHECK()
                        case GLTF_INT_8:
                            minMaxVal.int8 = int8_t(minMaxEle.GetInt());
                        case GLTF_INT_16:
                            minMaxVal.int16 = int16_t(minMaxEle.GetInt());
                        case GLTF_UINT_8:
                            minMaxVal.uint8 = uint8_t(minMaxEle.GetUint());
                        case GLTF_UINT_16:
                            minMaxVal.uint16 = uint16_t(minMaxEle.GetUint());
                        case GLTF_UINT_32:
                            minMaxVal.uint32 = minMaxEle.GetUint();
                        case GLTF_FLOAT_32:
                            minMaxVal.float32 = minMaxEle.GetFloat();
                    }

                    if (c_str_equal(jsonString, "min")) {
                        gltfAccessor.min.emplace_back(minMaxVal);
                    }
                    if (c_str_equal(jsonString, "max")) {
                        gltfAccessor.max.emplace_back(minMaxVal);
                    }
                }
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_asset(GltfAsset &outAsset, const rapidjson::Value &jsonValue) {
    ASSERT(jsonValue.IsObject())
    for (rapidjson::Value::ConstMemberIterator itr = jsonValue.MemberBegin(); itr != jsonValue.MemberEnd(); ++itr) {
        const char *jsonString = itr->name.GetString();
        log_verbose(MSG_CONVERTER, "\t%s\n", jsonString)

        if (c_str_equal(jsonString, "generator")) {
            ASSERT(itr->value.IsString())
            ASSERT(itr->value.GetStringLength() < GLTF_STR_GENERATOR_SIZE)
            sprintf(outAsset.generator, "%s", itr->value.GetString());
            continue;
        } else if (c_str_equal(jsonString, "version")) {
            ASSERT(itr->value.IsString())
            ASSERT(itr->value.GetStringLength() < GLTF_STR_VERSION_SIZE)
            sprintf(outAsset.version, "%s", itr->value.GetString());
            continue;
        }
        NOT_IMPLEMENTED()
    }
}

void dump_gltf_intermediate(const GltfIntermediate &gltfData) {
    printf("start: dumping gltf intermediate:\n\n");

    printf("Asset:\n");
    printf(" Generator: %s\n", gltfData.asset.generator);
    printf(" Version: %s\n", gltfData.asset.version);
    printf("\n");

    printf("Accessor Array: [%zu]\n", gltfData.accessors.size());
    for (const GltfAccessor &accessor: gltfData.accessors) {
        printf(" componentType: %u - %s\n", accessor.componentType, GltfComponentTypeLookup(accessor.componentType));
        printf(" accessorType: %u - %s - [ %u ]\n", accessor.accessorType, GltfAccessorTypeLookup(accessor.accessorType), GltfAccessorTypeSizeLookup(accessor.accessorType));
        printf(" bufferView: %u \n", accessor.bufferView);
        printf(" byteOffset: %u \n", accessor.byteOffset);
        printf(" count: %u\n", accessor.count);

        const auto PrintAccessor = [](const std::vector<GltfComponentType> &data, const GltfComponentTypesEnum componentType) -> void {
            for (size_t i = 0; i < data.size(); ++i) {
                switch (componentType) {
                    case GLTF_INT_8:
                        printf("%" PRIi8 " ", data[i].int8);
                        break;
                    case GLTF_INT_16:
                        printf("%" PRIi16 " ", data[i].int16);
                        break;
                    case GLTF_UINT_8:
                        printf("%" PRIu8 " ", data[i].uint8);
                        break;
                    case GLTF_UINT_16:
                        printf("%" PRIu16 " ", data[i].uint16);
                        break;
                    case GLTF_UINT_32:
                        printf("%" PRIu32 " ", data[i].uint32);
                        break;
                    case GLTF_FLOAT_32:
                        printf("%f ", data[i].float32);
                        break;
                    case GLTF_COMPONENT_UNDEFINED:
                    case GLTF_COMPONENT_UNUSED:
                    default: SANITY_CHECK()
                }
            }
        };

        printf(" min: ");
        PrintAccessor(accessor.min, accessor.componentType);
        printf("\n");

        printf(" max: ");
        PrintAccessor(accessor.max, accessor.componentType);
        printf("\n");
        printf("\n");
    }

    printf("bufferViews Array: [%zu]\n", gltfData.bufferViews.size());
    for (const GltfBufferViews &bufferViews: gltfData.bufferViews) {
        printf(" buffer: %u\n", bufferViews.buffer);
        printf(" byteOffset: %u\n", bufferViews.byteOffset);
        printf(" byteLength: %u \n", bufferViews.byteLength);
        printf(" target: %u \n", bufferViews.target);
        printf("\n");
    }

    printf("bufferViews Array: [%zu]\n", gltfData.textures.size());
    for (const GltfTextures &texture: gltfData.textures) {
        printf(" buffer: %u\n", texture.sampler);
        printf(" byteOffset: %u\n", texture.source);
        printf("\n");
    }

    printf("samplers Array: [%zu]\n", gltfData.samplers.size());
    for (const GltfSamplers &samplers: gltfData.samplers) {
        printf(" magFilter: %u\n", samplers.magFilter);
        printf(" minFilter: %u\n", samplers.minFilter);
        printf("\n");
    }

    printf("Images Array: [%zu]\n", gltfData.images.size());
    for (const GltfImages &images: gltfData.images) {
        printf(" bufferView: %u\n", images.bufferView);
        printf(" name: %s\n", images.name);
        printf(" mediaType: %s\n", images.mediaType);
        printf("\n");
    }


    printf("end: dumping gltf intermediate:\n");
    printf("\n");
}

void gltf_parse_json() {
    log_verbose(MSG_CONVERTER, "%s \n", gltfJson)

    rapidjson::Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.
    if (document.Parse(gltfJson).HasParseError()) {
        SANITY_CHECK()
    }

    ASSERT(document.IsObject())
    for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
        const char *jsonString = itr->name.GetString();
        if (c_str_equal(jsonString, "asset")) {
            parse_gltf_asset(g_gltfData.asset, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "accessors")) {
            parse_gltf_accessors(g_gltfData.accessors, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "bufferViews")) {
            parse_gltf_buffer_views(g_gltfData.bufferViews, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "textures")) {
            parse_gltf_textures(g_gltfData.textures, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "samplers")) {
            parse_gltf_samplers(g_gltfData.samplers, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "images")) {
            parse_gltf_images(g_gltfData.images, itr->value);
            continue;
        }

//        NOT_IMPLEMENTED(); Add this once I have the basics for GLTF parsing and just add the bits I need as an when I run into them.
    }

    dump_gltf_intermediate(g_gltfData);
}

//#define CGLTF_IMPLEMENTATION
//
//#include "../../../beet_pipeline/third/cgltf/cgltf.h"
//#include <vector>
//
//constexpr uint32_t accessorPoolSize = 500000; // 3 MiB
//static float s_accessorDataPool[accessorPoolSize] = {};

std::vector<GfxMesh> gfx_mesh_load_gltf() {
    gltf_parse_json();
    return {};


//    std::vector<GfxMesh> outMeshes;
//
//    //TODO: currently lit entities have a root to the world, need to fix this.
////    float nodeToWorld[16];
////    cgltf_node_transform_world(_node, nodeToWorld);
//    memset(s_accessorDataPool, 0, accessorPoolSize);
//
////    const char *path = "assets/scenes/intel/NewSponza_Main_glTF_002.gltf";
////    const char *path = "assets/scenes/glTF-Sample-Assets-main/Models/SciFiHelmet/glTF/SciFiHelmet.gltf";
//    const char *path = "assets/scenes/glTF-Sample-Assets-main/Models/Sponza/glTF/Sponza.gltf";
////    const char *path = "assets/scenes/glTF-Sample-Assets-main/Models/GlassVaseFlowers/glTF/GlassVaseFlowers.gltf";
//
//    cgltf_options options = {};
//    cgltf_data *data = nullptr;
//    cgltf_result result = cgltf_parse_file(&options, path, &data);
//    if (result == cgltf_result_success) {
//        result = cgltf_load_buffers(&options, data, path);
//        if (result == cgltf_result_success) {
//            for (size_t sceneIndex = 0; sceneIndex < data->scenes_count; ++sceneIndex) {
//                cgltf_scene *scene = &data->scenes[sceneIndex];
//                for (size_t nodeIndex = 0; nodeIndex < scene->nodes_count; ++nodeIndex) {
//                    cgltf_node *_node = scene->nodes[nodeIndex];
//                    cgltf_mesh *mesh = _node->mesh;
//                    if (mesh != nullptr) {
//                        for (size_t primitiveIndex = 0; primitiveIndex < mesh->primitives_count; ++primitiveIndex) {
//                            cgltf_primitive *primitive = &mesh->primitives[primitiveIndex];
//                            size_t numVertex = primitive->attributes[0].data->count;
//
//                            // Reserve space for data to avoid frequent reallocation
//                            std::vector<vec3f> position;
//                            std::vector<vec3f> normal;
//                            std::vector<vec2f> uv;
//                            std::vector<vec3f> color;
//                            std::vector<GfxVertex> vertices;
//                            std::vector<uint32_t> indices;
//
//                            int32_t basePositionIndex = 0;
//                            int32_t baseNormalIndex = 0;
//                            int32_t baseTexcoordIndex = 0;
//                            int32_t baseColorIndex = 0;
//
//                            bool hasNormal = false;
//                            bool hasTexcoord = false;
//                            bool hasColor = false;
//
//                            for (size_t attributeIndex = 0; attributeIndex < primitive->attributes_count; ++attributeIndex) {
//                                cgltf_attribute *attribute = &primitive->attributes[attributeIndex];
//                                cgltf_accessor *accessor = attribute->data;
//                                size_t accessorCount = accessor->count;
//
//                                size_t floatCount = cgltf_accessor_unpack_floats(accessor, nullptr, 0);
//                                cgltf_accessor_unpack_floats(accessor, s_accessorDataPool, floatCount);
//
//                                if (attribute->type == cgltf_attribute_type_position && attribute->index == 0) {
//                                    position.reserve(position.size() + accessorCount);
//                                    for (size_t v = 0; v < accessorCount; ++v) {
//                                        position.emplace_back(s_accessorDataPool[v * 3], s_accessorDataPool[v * 3 + 1], s_accessorDataPool[v * 3 + 2]);
//                                    }
//                                    basePositionIndex = position.size() - accessorCount;
//                                } else if (attribute->type == cgltf_attribute_type_normal && attribute->index == 0) {
//                                    normal.reserve(normal.size() + accessorCount);
//                                    for (size_t v = 0; v < accessorCount; ++v) {
//                                        normal.emplace_back(s_accessorDataPool[v * 3], s_accessorDataPool[v * 3 + 1], s_accessorDataPool[v * 3 + 2]);
//                                    }
//                                    baseNormalIndex = normal.size() - accessorCount;
//                                    hasNormal = true;
//                                } else if (attribute->type == cgltf_attribute_type_texcoord && attribute->index == 0) {
//                                    uv.reserve(uv.size() + accessorCount);
//                                    for (size_t v = 0; v < accessorCount; ++v) {
//                                        uv.emplace_back(s_accessorDataPool[v * 2], s_accessorDataPool[v * 2 + 1]);
//                                    }
//                                    baseTexcoordIndex = uv.size() - accessorCount;
//                                    hasTexcoord = true;
//                                } else if (attribute->type == cgltf_attribute_type_color && attribute->index == 0) {
//                                    color.reserve(color.size() + accessorCount);
//                                    for (size_t v = 0; v < accessorCount; ++v) {
//                                        color.emplace_back(s_accessorDataPool[v * 3], s_accessorDataPool[v * 3 + 1], s_accessorDataPool[v * 3 + 2]);
//                                    }
//                                    baseColorIndex = color.size() - accessorCount;
//                                    hasColor = true;
//                                }
//
//                            }
//
//                            if (primitive->indices != nullptr) {
//                                cgltf_accessor *accessor = primitive->indices;
//                                size_t indexCount = accessor->count;
//
//                                for (size_t v = 0; v < indexCount; v += 3) {
//                                    for (int i = 0; i < 3; ++i) {
//                                        size_t vertexIndex = cgltf_accessor_read_index(accessor, v + i);
//                                        GfxVertex gfxVertex = {};
//                                        gfxVertex.pos = position[basePositionIndex + vertexIndex];
//                                        gfxVertex.normal = hasNormal ? normal[baseNormalIndex + vertexIndex] : vec3f{0.0f, 0.0f, 0.0f};
//                                        gfxVertex.uv = hasTexcoord ? uv[baseTexcoordIndex + vertexIndex] : vec2f{0.0f, 0.0f};
//                                        gfxVertex.color = hasColor ? color[baseColorIndex + vertexIndex] : vec3f{1.0f, 1.0f, 1.0f}; // Default white color
//
//                                        vertices.push_back(gfxVertex);
//                                        indices.push_back(static_cast<uint32_t>(vertices.size() - 1));
//                                    }
//                                }
//                            }
//
//                            const RawMesh rawMesh = {
//                                    vertices.data(),
//                                    indices.data(),
//                                    static_cast<uint32_t>(vertices.size()),
//                                    static_cast<uint32_t>(indices.size()),
//                            };
//
//                            GfxMesh outMesh = {};
//                            gfx_mesh_create_immediate(rawMesh, outMesh);
//                            outMeshes.push_back(outMesh);
//                        }
//                    }
//                }
//            }
//        }
//        cgltf_free(data);
//    }
//
//    return outMeshes;
}
#endif //IN_DEV_RUNTIME_GLTF_LOADING

void gfx_mesh_create_cube_immediate(GfxMesh &outMesh) {
    const uint32_t vertexCount = 24;
    GfxVertex vertexData[vertexCount] = {
            //===POS================//===NORMAL=========//===UV======//===COLOUR=========
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    };

    const uint32_t indexCount = 36;
    uint32_t indexData[indexCount] = {
            0, 3, 2,
            2, 1, 0,
            4, 5, 6,
            6, 7, 4,
            11, 8, 9,
            9, 10, 11,
            12, 13, 14,
            14, 15, 12,
            16, 17, 18,
            18, 19, 16,
            20, 21, 22,
            22, 23, 20
    };

    const RawMesh rawMesh = {
            vertexData,
            indexData,
            vertexCount,
            indexCount,
    };

    gfx_mesh_create_immediate(rawMesh, outMesh);
}

void gfx_mesh_cleanup(GfxMesh &mesh) {
    vkDestroyBuffer(g_vulkanBackend.device, mesh.vertBuffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, mesh.vertMemory, nullptr);
    vkDestroyBuffer(g_vulkanBackend.device, mesh.indexBuffer, nullptr);
    vkFreeMemory(g_vulkanBackend.device, mesh.indexMemory, nullptr);
    mesh = {};

    //TODO:GFX We don't re-add this as a free slot in the texture pool i.e.
    //we could address this pretty simply in a few ways.
    //create free-list for each pool and next time we try and create a new texture to check if any free list spaces are free
    //we move the last image loaded into the newly free position and fix up and dependency, this will break any cached references.
}
//======================================================================================================================