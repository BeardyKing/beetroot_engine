#include <runtime/entity_builder.h>

#include <beet_gfx/vulkan/gfx_vk_texture.h>
#include <beet_gfx/vulkan/gfx_vk_lit.h>
#include <beet_gfx/vulkan/gfx_vk_sky.h>
#include <beet_gfx/vulkan/gfx_vk_samplers.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/db_asset.h>

#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>
#include <beet_shared/c_string.h>

static struct {
    struct {
        uint32_t black = {UINT32_MAX};
        uint32_t white = {UINT32_MAX};
        uint32_t uvGrid = {UINT32_MAX};
        uint32_t skybox = {UINT32_MAX};
    } texture;

    struct {
        uint32_t cube = {UINT32_MAX};
        uint32_t octahedron = {UINT32_MAX};
    } mesh;

    struct {
        uint32_t camera = {UINT32_MAX};
        uint32_t skybox = {UINT32_MAX};
    } entity;

    struct {
        uint32_t skybox = {UINT32_MAX};
        uint32_t cube = {UINT32_MAX};
    } material;
} s_builtIn;

//===INTERNAL_FUNCTIONS=================================================================================================
static void built_in_primary_camera_create() {
    s_builtIn.entity.camera = db_add_camera_entity((CameraEntity) {
            .transformIndex = db_add_transform((Transform) {
                    .position{-1.5f, 0.5f, 2.5f},
                    .rotation{-0.2f, -0.65f, 0.0f}
            }),
            .cameraIndex = db_add_camera((Camera) {
                    .fov = 65, .zFar = 6000
            }),
    });
}

uint32_t add_texture_to_database(const char *texturePath, GfxTexture &texture) {
    gfx_texture_create_immediate_dds(texturePath, texture);
    return db_add_texture(texture);
}

static void built_in_textures_create() {
    {
        GfxTexture blackTexture = {};
        s_builtIn.texture.black = add_texture_to_database("assets/textures/black.dds", blackTexture);
    }
    {
        GfxTexture whiteTexture = {};
        s_builtIn.texture.white = add_texture_to_database("assets/textures/white.dds", whiteTexture);
    }
    {
        GfxTexture uvGridTexture = {};
        s_builtIn.texture.uvGrid = add_texture_to_database("assets/textures/UV_Grid/UV_Grid_test.dds", uvGridTexture);
    }
    {
        GfxTexture skyboxTexture = {.imageSamplerType = TextureSamplerType::LinearMirror};
        s_builtIn.texture.skybox = add_texture_to_database("assets/textures/sky/herkulessaulen_4k-octahedral.dds", skyboxTexture);
    }
}

static void built_in_meshes_create() {
    {
        GfxMesh octahedronMesh = {};
        gfx_mesh_create_octahedron_immediate(octahedronMesh);
        s_builtIn.mesh.octahedron = db_add_mesh(octahedronMesh);
    }
    {
        GfxMesh cubeMesh = {};
        gfx_mesh_create_cube_immediate(cubeMesh);
        s_builtIn.mesh.cube = db_add_mesh(cubeMesh);
    }
}

static void built_in_materials_create() {
    {
        VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
        gfx_sky_update_material_descriptor(descriptorSet, *db_get_texture(s_builtIn.texture.skybox));
        s_builtIn.material.skybox = db_add_sky_material({.descriptorSetIndex = db_add_descriptor_set(descriptorSet), .octahedralMapIndex = s_builtIn.texture.skybox});
    }
    {
        const GfxTexture white = *db_get_texture(s_builtIn.texture.white);
        VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
        gfx_lit_update_material_descriptor(descriptorSet, white, white, white, white, white);
        s_builtIn.material.cube = db_add_lit_material({.descriptorSetIndex = db_add_descriptor_set(descriptorSet)});
    }
}

static void built_in_entities_create() {
    s_builtIn.entity.skybox = db_add_sky_entity((SkyEntity) {
            .meshIndex = s_builtIn.mesh.octahedron,
            .materialIndex = s_builtIn.material.skybox,
    });
}

static bool load_package(const char *packagePath) {
#if CHECK_FEATURE(FEATURE_IN_DEV_RUNTIME_GLTF_LOADING)
    const AssetPackage package = asset_package_load_gltf(packagePath);
    for (const PackageEntry &assetEntry: package.packageTable) {
        // TODO: AssetPackage should contain a list of textures to upload the table should point to the index,
        // currently we upload a texture per material i.e. lots of duplication of textures,
        // I was mainly interested in validating my GLTF parser was working correctly before sorting this issue.

        const RawMaterial &packageMaterial = package.materials[assetEntry.materialIndex];
        GfxTexture albedoTexture = {};
        GfxTexture normalTexture = {};
        GfxTexture metallicRoughnessTexture = {};
        GfxTexture occlusionTexture = {};
        GfxTexture emissiveTexture = {};

        // Load and add textures to the database
        uint32_t albedoIndex = add_texture_to_database(packageMaterial.albedoPath, albedoTexture);
        uint32_t normalIndex = add_texture_to_database(packageMaterial.normalMapPath, normalTexture);
        uint32_t metallicRoughnessIndex = add_texture_to_database(packageMaterial.metallicRoughnessPath, metallicRoughnessTexture);
        uint32_t occlusionIndex = add_texture_to_database(packageMaterial.occlusionMapPath, occlusionTexture);
        uint32_t emissiveIndex = add_texture_to_database(packageMaterial.emissiveMapPath, emissiveTexture);

        // Update descriptor set with all textures
        VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
        gfx_lit_update_material_descriptor(
                descriptorSet,
                *db_get_texture(albedoIndex),
                *db_get_texture(normalIndex),
                *db_get_texture(metallicRoughnessIndex),
                *db_get_texture(occlusionIndex),
                *db_get_texture(emissiveIndex)
        );


        uint32_t litEntity = db_add_lit_entity((LitEntity) {
                .transformIndex = db_add_transform((Transform) {
                        .position = {0.0f, 0.0f, 0.0f}, .rotation = {90.0f, 0.0f, 0.0f}, .scale = {1.0f, 1.0f, 1.0f}
                }),

                .meshIndex = db_add_mesh((GfxMesh) {
                        package.meshes[assetEntry.meshIndex]
                }),

                .materialIndex = db_add_lit_material((LitMaterial) {
                        .descriptorSetIndex = db_add_descriptor_set(descriptorSet),
                        .albedoIndex = {}
                }),
        });
#if BEET_DEBUG
        char debugName[DEBUG_NAME_MAX] = {};
        if (c_string_extract_file_name(packagePath, debugName)) {
            sprintf(db_get_lit_entity(litEntity)->debug_name, "%s", debugName);
        }
#endif //BEET_DEBUG
    }
#endif //IN_DEV_RUNTIME_GLTF_LOADING
    return true;
}
//======================================================================================================================

uint32_t entity_create_lit_cube() {
    return db_add_lit_entity((LitEntity) {
            .transformIndex = db_add_transform((Transform) {}),
            .meshIndex = s_builtIn.mesh.cube,
            .materialIndex = s_builtIn.material.cube,
    });
}

//===INIT_&_SHUTDOWN====================================================================================================
void entities_cleanup() {
    // if we are shutting down we can ignore all entities created in `primary_camera_entity_create`
    // but as a follow-up I should add some way to reset / free up various entities in the db
    // this would mainly be for an editor workflow as I would want a dedicated function to load/unload a package

    // itr gfx data and invalidate content
    for (uint32_t i = 0; i < db_get_mesh_count(); ++i) {
        gfx_mesh_cleanup(*db_get_mesh(i));
    }
    for (uint32_t i = 0; i < db_get_texture_count(); ++i) {
        gfx_texture_cleanup(*db_get_texture(i));
    }
}

void entities_create() {
    built_in_primary_camera_create();
    built_in_textures_create();
    built_in_meshes_create();
    built_in_materials_create();
    built_in_entities_create();

    ASSERT(load_package("assets/scenes/glTF-Sample-Assets/Models/DamagedHelmet/glTF/DamagedHelmet.gltf"));
//    ASSERT(load_package("assets/scenes/glTF-Sample-Assets/Models/FlightHelmet/glTF/FlightHelmet.gltf"));
//    ASSERT(load_package("assets/scenes/glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf"));
}
//======================================================================================================================
