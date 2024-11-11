#include <beet_gfx/vulkan/gfx_vk_mesh.h>
#include <beet_gfx/vulkan/gfx_vk_buffer.h>
#include <beet_gfx/vulkan/gfx_vk_command.h>
#include <beet_gfx/gfx_types.h>

#include <beet_shared/assert.h>

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
            {{+0.0f * 100, -0.0f * 100, +1.0f * 100}, {+0.577350f, -0.577350f, +0.577350f}, {0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, -1.0f * 100, -0.0f * 100}, {+0.577350f, -0.577350f, +0.577350f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+1.0f * 100, +0.0f * 100, +0.0f * 100}, {+0.577350f, -0.577350f, +0.577350f}, {0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+1.0f * 100, +0.0f * 100, +0.0f * 100}, {+0.577350f, -0.577350f, -0.577350f}, {0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, -1.0f * 100, -0.0f * 100}, {+0.577350f, -0.577350f, -0.577350f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, +0.0f * 100, -1.0f * 100}, {+0.577350f, -0.577350f, -0.577350f}, {0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, +0.0f * 100, -1.0f * 100}, {-0.577350f, -0.577350f, -0.577350f}, {0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, -1.0f * 100, -0.0f * 100}, {-0.577350f, -0.577350f, -0.577350f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-1.0f * 100, +0.0f * 100, +0.0f * 100}, {-0.577350f, -0.577350f, -0.577350f}, {1.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{-1.0f * 100, +0.0f * 100, +0.0f * 100}, {-0.577350f, -0.577350f, +0.577350f}, {1.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, -1.0f * 100, -0.0f * 100}, {-0.577350f, -0.577350f, +0.577350f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, -0.0f * 100, +1.0f * 100}, {-0.577350f, -0.577350f, +0.577350f}, {0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, +0.0f * 100, -1.0f * 100}, {+0.577350f, +0.577350f, -0.577350f}, {0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, +1.0f * 100, +0.0f * 100}, {+0.577350f, +0.577350f, -0.577350f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+1.0f * 100, +0.0f * 100, +0.0f * 100}, {+0.577350f, +0.577350f, -0.577350f}, {0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{-1.0f * 100, +0.0f * 100, +0.0f * 100}, {-0.577350f, +0.577350f, -0.577350f}, {1.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, +1.0f * 100, +0.0f * 100}, {-0.577350f, +0.577350f, -0.577350f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, +0.0f * 100, -1.0f * 100}, {-0.577350f, +0.577350f, -0.577350f}, {0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, -0.0f * 100, +1.0f * 100}, {-0.577350f, +0.577350f, +0.577350f}, {0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, +1.0f * 100, +0.0f * 100}, {-0.577350f, +0.577350f, +0.577350f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{-1.0f * 100, +0.0f * 100, +0.0f * 100}, {-0.577350f, +0.577350f, +0.577350f}, {1.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+1.0f * 100, +0.0f * 100, +0.0f * 100}, {+0.577350f, +0.577350f, +0.577350f}, {0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, +1.0f * 100, +0.0f * 100}, {+0.577350f, +0.577350f, +0.577350f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{+0.0f * 100, -0.0f * 100, +1.0f * 100}, {+0.577350f, +0.577350f, +0.577350f}, {0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
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

#if CHECK_FEATURE(FEATURE_IN_DEV_RUNTIME_GLTF_LOADING)

#include <beet_shared/filesystem.h>
#include <beet_shared/memory.h>
#include <beet_shared/c_string.h>
#include <beet_shared/log.h>

#include <beet_math/quat.h>

#define CGLTF_IMPLEMENTATION

#include "../../../third_party/cgltf/cgltf.h"


#include <beet_gfx/gfx_converter.h>
#include <vector>

#if CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)

#include <beet_converter/converter_types.h>

extern ConverterLocations g_converterLocations;
#endif //CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)

void set_material_image_path(char *outPath, char *inURI, const char *targetWriteDirectory) {
    if (inURI) {
        strcpy(outPath, targetWriteDirectory);
        char *lastDir = c_str_search_reverse(outPath, "/") + 1;
        strcpy(lastDir, inURI);
        c_string_replace_extension(outPath, ".dds");
    } else {
        sprintf(outPath, "%s", "white.dds");
        log_verbose(MSG_CONVERTER, "using fallback texture for asset %s", targetWriteDirectory);
    }
}

void Process_material(const cgltf_material &material, RawMaterial &rawMaterial, const char *targetWriteDirectory) {
    if (material.pbr_metallic_roughness.base_color_texture.texture) {
        const cgltf_image *image = material.pbr_metallic_roughness.base_color_texture.texture->image;
        set_material_image_path(rawMaterial.albedoPath, image->uri, targetWriteDirectory);

        rawMaterial.albedoCoords.x = material.pbr_metallic_roughness.base_color_texture.texcoord;
        rawMaterial.albedoCoords.y = material.pbr_metallic_roughness.base_color_texture.texcoord;
    }

    if (material.normal_texture.texture) {
        const cgltf_image *image = material.normal_texture.texture->image;
        set_material_image_path(rawMaterial.normalMapPath, image->uri, targetWriteDirectory);

        rawMaterial.normalMapCoords.x = material.normal_texture.texcoord;
        rawMaterial.normalMapCoords.y = material.normal_texture.texcoord;
        rawMaterial.normalScale = material.normal_texture.scale;
    }

    if (material.pbr_metallic_roughness.metallic_roughness_texture.texture) {
        const cgltf_image *image = material.pbr_metallic_roughness.metallic_roughness_texture.texture->image;
        set_material_image_path(rawMaterial.metallicRoughnessPath, image->uri, targetWriteDirectory);

        rawMaterial.metallicRoughnessCoords.x = material.pbr_metallic_roughness.metallic_roughness_texture.texcoord;
        rawMaterial.metallicRoughnessCoords.y = material.pbr_metallic_roughness.metallic_roughness_texture.texcoord;
        rawMaterial.metallicFactor = material.pbr_metallic_roughness.metallic_factor;
        rawMaterial.roughnessFactor = material.pbr_metallic_roughness.roughness_factor;
    }

    if (material.occlusion_texture.texture) {
        const cgltf_image *image = material.occlusion_texture.texture->image;
        set_material_image_path(rawMaterial.occlusionMapPath, image->uri, targetWriteDirectory);

        rawMaterial.occlusionCoords.x = material.occlusion_texture.texcoord;
        rawMaterial.occlusionCoords.y = material.occlusion_texture.texcoord;
        rawMaterial.occlusionStrength = material.occlusion_texture.scale;
    }

    if (material.emissive_texture.texture) {
        const cgltf_image *image = material.emissive_texture.texture->image;
        set_material_image_path(rawMaterial.emissiveMapPath, image->uri, targetWriteDirectory);

        rawMaterial.emissiveCoords.x = material.emissive_texture.texcoord;
        rawMaterial.emissiveCoords.y = material.emissive_texture.texcoord;
        rawMaterial.emissiveFactor[0] = material.emissive_factor[0];
        rawMaterial.emissiveFactor[1] = material.emissive_factor[1];
        rawMaterial.emissiveFactor[2] = material.emissive_factor[2];
    }
}

void load_gltf_file(const char *filePath, const char *targetWriteDirectory, AssetPackage &package) {
    cgltf_options options = {};
    cgltf_data *data = nullptr;

    // Load the GLTF file
    cgltf_result result = cgltf_parse_file(&options, filePath, &data);
    if (result != cgltf_result_success) {
        ASSERT_MSG(false, "Failed to load GLTF file.");
        return;
    }

    // Load buffers
    result = cgltf_load_buffers(&options, data, filePath);
    if (result != cgltf_result_success) {
        ASSERT_MSG(false, "Failed to load GLTF buffers.");
        cgltf_free(data);
        return;
    }

    // Process meshes
    for (size_t meshIndex = 0; meshIndex < data->meshes_count; ++meshIndex) {
        const cgltf_mesh &mesh = data->meshes[meshIndex];
        for (size_t primIndex = 0; primIndex < mesh.primitives_count; ++primIndex) {
            const cgltf_primitive &primitive = mesh.primitives[primIndex];

            std::vector<GfxVertex> raw_verts;
            std::vector<uint32_t> raw_indices;

            // Process vertex data
            for (size_t j = 0; j < primitive.attributes_count; ++j) {
                const cgltf_attribute &attr = primitive.attributes[j];

                if (attr.type == cgltf_attribute_type_position) {
                    const cgltf_accessor *accessor = attr.data;
                    raw_verts.resize(accessor->count);

                    for (size_t v = 0; v < accessor->count; ++v) {
                        cgltf_accessor_read_float(accessor, v, &raw_verts[v].pos[0], 3);
                    }
                } else if (attr.type == cgltf_attribute_type_normal) {
                    const cgltf_accessor *accessor = attr.data;
                    raw_verts.resize(accessor->count);

                    for (size_t v = 0; v < accessor->count; ++v) {
                        cgltf_accessor_read_float(accessor, v, &raw_verts[v].normal[0], 3);
                    }
                } else if (attr.type == cgltf_attribute_type_texcoord) {
                    const cgltf_accessor *accessor = attr.data;
                    raw_verts.resize(accessor->count);

                    for (size_t v = 0; v < accessor->count; ++v) {
                        cgltf_accessor_read_float(accessor, v, &raw_verts[v].uv[0], 2);
                    }
                }
            }

            // Process index data
            const cgltf_accessor *indexAccessor = primitive.indices;
            raw_indices.resize(indexAccessor->count);

            for (size_t k = 0; k < indexAccessor->count; ++k) {
                raw_indices[k] = static_cast<uint32_t>(cgltf_accessor_read_index(indexAccessor, k));
            }

            // Construct the RawMesh
            RawMesh rawMesh = {
                    raw_verts.data(),
                    raw_indices.data(),
                    static_cast<uint32_t>(raw_verts.size()),
                    static_cast<uint32_t>(raw_indices.size())
            };

            // Add the RawMesh to the AssetPackage
            GfxMesh &outMesh = package.meshes.emplace_back(); // maybe this should be a RawMesh instead. then we cal call create immediate from entity_builder.
            gfx_mesh_create_immediate(rawMesh, outMesh);

            // Create a package entry
            PackageEntry entry = {};
            entry.meshIndex = static_cast<uint32_t>(package.meshes.size() - 1);
            entry.materialIndex = primitive.material - data->materials;

            package.packageTable.push_back(entry);
        }
    }

    // Process materials
    for (size_t i = 0; i < data->materials_count; ++i) {
        const cgltf_material &material = data->materials[i];
        RawMaterial rawMaterial = {};
        Process_material(material, rawMaterial, targetWriteDirectory);
        package.materials.push_back(rawMaterial);
    }

    cgltf_free(data);
}


AssetPackage asset_package_load_gltf(const char *inPath) {
    char path[128] = {};
    char writePath[128] = {};
    c_string_remove_file_from_path(inPath, writePath);
#if CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)
    sprintf(path, "%s%s", g_converterLocations.rawAssetDir.c_str(), inPath);
    ASSERT(fs_file_exists(path));
#else
    sprintf(path, "%s", inPath);
#endif //CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)

    AssetPackage outPackage;
    load_gltf_file(path, writePath, outPackage);

    return outPackage;
}

#endif //IN_DEV_RUNTIME_GLTF_LOADING
void gfx_mesh_create_cube_immediate(GfxMesh &outMesh) {
    const uint32_t vertexCount = 24;
    GfxVertex vertexData[vertexCount] = {
            //===POS================//===NORMAL=========//===UV======//===COLOUR=========
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{-0.5f, -0.5f, +0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{-0.5f, +0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{+0.5f, +0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
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