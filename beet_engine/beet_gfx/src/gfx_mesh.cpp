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
    ASSERT((rawMesh.vertexCount > 0) && (rawMesh.indexCount > 0));

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
    ASSERT(vertexCreateStageRes == VK_SUCCESS);

    VkResult indexCreateStageRes = gfx_buffer_create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            indexBufferSize,
            indexStaging.buffer,
            indexStaging.memory,
            rawMesh.indexData
    );
    ASSERT(indexCreateStageRes == VK_SUCCESS);

    // Create device local buffers
    const VkResult vertexCreateDeviceLocalRes = gfx_buffer_create(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBufferSize,
            outMesh.vertBuffer,
            outMesh.vertMemory,
            nullptr
    );
    ASSERT(vertexCreateDeviceLocalRes == VK_SUCCESS);
    // Index buffer
    const VkResult indexCreateDeviceLocalRes = gfx_buffer_create(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indexBufferSize,
            outMesh.indexBuffer,
            outMesh.indexMemory,
            nullptr
    );
    ASSERT(indexCreateDeviceLocalRes == VK_SUCCESS);

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
#define CGLTF_IMPLEMENTATION

#include "../../../beet_pipeline/third/cgltf/cgltf.h"
#include <vector>

constexpr uint32_t accessorPoolSize = 500000; // 3 MiB
static float s_accessorDataPool[accessorPoolSize] = {};

std::vector<GfxMesh> gfx_mesh_load_gltf() {
    std::vector<GfxMesh> outMeshes;

    //TODO: currently lit entities have a root to the world, need to fix this.
//    float nodeToWorld[16];
//    cgltf_node_transform_world(_node, nodeToWorld);
    memset(s_accessorDataPool, 0, accessorPoolSize);

//    const char *path = "assets/scenes/intel/NewSponza_Main_glTF_002.gltf";
//    const char *path = "assets/scenes/glTF-Sample-Assets-main/Models/SciFiHelmet/glTF/SciFiHelmet.gltf";
    const char *path = "assets/scenes/glTF-Sample-Assets-main/Models/Sponza/glTF/Sponza.gltf";
//    const char *path = "assets/scenes/glTF-Sample-Assets-main/Models/GlassVaseFlowers/glTF/GlassVaseFlowers.gltf";

    cgltf_options options = {};
    cgltf_data *data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, path, &data);
    if (result == cgltf_result_success) {
        result = cgltf_load_buffers(&options, data, path);
        if (result == cgltf_result_success) {
            for (size_t sceneIndex = 0; sceneIndex < data->scenes_count; ++sceneIndex) {
                cgltf_scene *scene = &data->scenes[sceneIndex];
                for (size_t nodeIndex = 0; nodeIndex < scene->nodes_count; ++nodeIndex) {
                    cgltf_node *_node = scene->nodes[nodeIndex];
                    cgltf_mesh *mesh = _node->mesh;
                    if (mesh != nullptr) {
                        for (size_t primitiveIndex = 0; primitiveIndex < mesh->primitives_count; ++primitiveIndex) {
                            cgltf_primitive *primitive = &mesh->primitives[primitiveIndex];
                            size_t numVertex = primitive->attributes[0].data->count;

                            // Reserve space for data to avoid frequent reallocation
                            std::vector<vec3f> position;
                            std::vector<vec3f> normal;
                            std::vector<vec2f> uv;
                            std::vector<vec3f> color;
                            std::vector<GfxVertex> vertices;
                            std::vector<uint32_t> indices;

                            int32_t basePositionIndex = 0;
                            int32_t baseNormalIndex = 0;
                            int32_t baseTexcoordIndex = 0;
                            int32_t baseColorIndex = 0;

                            bool hasNormal = false;
                            bool hasTexcoord = false;
                            bool hasColor = false;

                            for (size_t attributeIndex = 0; attributeIndex < primitive->attributes_count; ++attributeIndex) {
                                cgltf_attribute *attribute = &primitive->attributes[attributeIndex];
                                cgltf_accessor *accessor = attribute->data;
                                size_t accessorCount = accessor->count;

                                size_t floatCount = cgltf_accessor_unpack_floats(accessor, nullptr, 0);
                                cgltf_accessor_unpack_floats(accessor, s_accessorDataPool, floatCount);

                                if (attribute->type == cgltf_attribute_type_position && attribute->index == 0) {
                                    position.reserve(position.size() + accessorCount);
                                    for (size_t v = 0; v < accessorCount; ++v) {
                                        position.emplace_back(s_accessorDataPool[v * 3], s_accessorDataPool[v * 3 + 1], s_accessorDataPool[v * 3 + 2]);
                                    }
                                    basePositionIndex = position.size() - accessorCount;
                                } else if (attribute->type == cgltf_attribute_type_normal && attribute->index == 0) {
                                    normal.reserve(normal.size() + accessorCount);
                                    for (size_t v = 0; v < accessorCount; ++v) {
                                        normal.emplace_back(s_accessorDataPool[v * 3], s_accessorDataPool[v * 3 + 1], s_accessorDataPool[v * 3 + 2]);
                                    }
                                    baseNormalIndex = normal.size() - accessorCount;
                                    hasNormal = true;
                                } else if (attribute->type == cgltf_attribute_type_texcoord && attribute->index == 0) {
                                    uv.reserve(uv.size() + accessorCount);
                                    for (size_t v = 0; v < accessorCount; ++v) {
                                        uv.emplace_back(s_accessorDataPool[v * 2], s_accessorDataPool[v * 2 + 1]);
                                    }
                                    baseTexcoordIndex = uv.size() - accessorCount;
                                    hasTexcoord = true;
                                } else if (attribute->type == cgltf_attribute_type_color && attribute->index == 0) {
                                    color.reserve(color.size() + accessorCount);
                                    for (size_t v = 0; v < accessorCount; ++v) {
                                        color.emplace_back(s_accessorDataPool[v * 3], s_accessorDataPool[v * 3 + 1], s_accessorDataPool[v * 3 + 2]);
                                    }
                                    baseColorIndex = color.size() - accessorCount;
                                    hasColor = true;
                                }

                            }

                            if (primitive->indices != nullptr) {
                                cgltf_accessor *accessor = primitive->indices;
                                size_t indexCount = accessor->count;

                                for (size_t v = 0; v < indexCount; v += 3) {
                                    for (int i = 0; i < 3; ++i) {
                                        size_t vertexIndex = cgltf_accessor_read_index(accessor, v + i);
                                        GfxVertex gfxVertex = {};
                                        gfxVertex.pos = position[basePositionIndex + vertexIndex];
                                        gfxVertex.normal = hasNormal ? normal[baseNormalIndex + vertexIndex] : vec3f{0.0f, 0.0f, 0.0f};
                                        gfxVertex.uv = hasTexcoord ? uv[baseTexcoordIndex + vertexIndex] : vec2f{0.0f, 0.0f};
                                        gfxVertex.color = hasColor ? color[baseColorIndex + vertexIndex] : vec3f{1.0f, 1.0f, 1.0f}; // Default white color

                                        vertices.push_back(gfxVertex);
                                        indices.push_back(static_cast<uint32_t>(vertices.size() - 1));
                                    }
                                }
                            }

                            const RawMesh rawMesh = {
                                    vertices.data(),
                                    indices.data(),
                                    static_cast<uint32_t>(vertices.size()),
                                    static_cast<uint32_t>(indices.size()),
                            };

                            GfxMesh outMesh = {};
                            gfx_mesh_create_immediate(rawMesh, outMesh);
                            outMeshes.push_back(outMesh);
                        }
                    }
                }
            }
        }
        cgltf_free(data);
    }

    return outMeshes;
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