#include <runtime/entity_builder.h>

#include <beet_gfx/vulkan/gfx_vk_texture.h>
#include <beet_gfx/vulkan/gfx_vk_lit.h>
#include <beet_gfx/vulkan/gfx_vk_sky.h>
#include <beet_gfx/vulkan/gfx_vk_samplers.h>
#include <beet_gfx/gfx_types.h>
#include <beet_gfx/db_asset.h>

#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>

//===INTERNAL_FUNCTIONS=================================================================================================
static void primary_camera_entity_create() {
    const CameraEntity cameraEntity{
            .transformIndex = db_add_transform(
                    {
                            .position{-1.5f, 0.5f, 2.5f},
                            .rotation{-0.2f, -0.65f, 0.0f}
                    }
            ),
            .cameraIndex = db_add_camera({.fov = 65, .zFar = 6000}),
    };
    db_add_camera_entity(cameraEntity);
}
uint32_t add_texture_to_database(const char *texturePath, GfxTexture &texture) {
    gfx_texture_create_immediate_dds(texturePath, texture);
    return db_add_texture(texture);
}
static void lit_entities_create() {
    //===PACKAGE==================================================
#if CHECK_FEATURE(FEATURE_IN_DEV_RUNTIME_GLTF_LOADING)
    const AssetPackage package = asset_package_load_gltf("assets/scenes/glTF-Sample-Assets/Models/DamagedHelmet/glTF/DamagedHelmet.gltf");
//    const AssetPackage package = asset_package_load_gltf("assets/scenes/glTF-Sample-Assets/Models/FlightHelmet/glTF/FlightHelmet.gltf");
//    const AssetPackage package = asset_package_load_gltf("assets/scenes/glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf");
    for (const PackageEntry &assetEntry: package.packageTable) {
        // TODO: AssetPackage should contain a list of textures to upload the table should point to the index,
        // currently we upload a texture per material i.e. lots of duplication of textures,
        // I was mainly interested in validating my GLTF parser was working correctly before sorting this issue.
        const GfxMesh &packageMesh = package.meshes[assetEntry.meshIndex];
        const uint32_t db_meshIndex = db_add_mesh(packageMesh);

        Transform defaultTransform = {.position = {0.0f, 0.0f, 0.0f}, .rotation = {90.0f, 0.0f, 0.0f}, .scale = {1.0f, 1.0f, 1.0f}};
        const uint32_t db_transformIndex = db_add_transform(defaultTransform);

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


        VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
        // Update descriptor set with all textures
        gfx_lit_update_material_descriptor(
                descriptorSet,
                *db_get_texture(albedoIndex),
                *db_get_texture(normalIndex),
                *db_get_texture(metallicRoughnessIndex),
                *db_get_texture(occlusionIndex),
                *db_get_texture(emissiveIndex)
        );
        const LitMaterial material = {
                .descriptorSetIndex = db_add_descriptor_set(descriptorSet),
                .albedoIndex = {}
        };

        uint32_t db_litMaterialIndex = db_add_lit_material(material);

        const LitEntity defaultCube = {
                .transformIndex = db_transformIndex,
                .meshIndex = db_meshIndex,
                .materialIndex = db_litMaterialIndex,
        };
        db_add_lit_entity(defaultCube);
    }
#endif //IN_DEV_RUNTIME_GLTF_LOADING
    //===MESH=====================================================
    uint32_t cubeID = {UINT32_MAX};
    {
        GfxMesh cubeMesh = {};
        gfx_mesh_create_cube_immediate(cubeMesh);
        cubeID = db_add_mesh(cubeMesh);
    }

    uint32_t octahedronID = {UINT32_MAX};
    {
        // const AssetPackage octahedronPackage = asset_package_load_gltf("assets/scenes/shapes/octahedron.gltf");
        // ASSERT(octahedronPackage.packageTable.size() == 1)
        // octahedronID = db_add_mesh(octahedronPackage.meshes[0]);
        GfxMesh octahedronMesh = {};
        gfx_mesh_create_octahedron_immediate(octahedronMesh);
        octahedronID = db_add_mesh(octahedronMesh);
    }
    //============================================================

    //===TEXTURE==================================================
    uint32_t uvGridTextureID = {UINT32_MAX};
    {
        GfxTexture uvTestTexture = {};
        gfx_texture_create_immediate_dds("assets/textures/UV_Grid/UV_Grid_test.dds", uvTestTexture);
        uvGridTextureID = db_add_texture(uvTestTexture);
    }
    uint32_t skyboxTextureID = {UINT32_MAX};
    {
        GfxTexture skyboxTexture = {.imageSamplerType = TextureSamplerType::LinearMirror};
        gfx_texture_create_immediate_dds("assets/textures/sky/herkulessaulen_4k-octahedral.dds", skyboxTexture);
        skyboxTextureID = db_add_texture(skyboxTexture);
    }
    //============================================================

    //===MATERIAL=================================================
//    uint32_t cubeLitMaterialID = {UINT32_MAX};
//    {
//        VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
//        gfx_lit_update_material_descriptor(descriptorSet, *db_get_texture(uvGridTextureID));
//
//        const LitMaterial material = {
//                .descriptorSetIndex = db_add_descriptor_set(descriptorSet),
//                .albedoIndex = uvGridTextureID
//        };
//
//        cubeLitMaterialID = db_add_lit_material(material);
//    }

    uint32_t octaSkyMaterialID = {UINT32_MAX};
    {
        VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
        gfx_sky_update_material_descriptor(descriptorSet, *db_get_texture(skyboxTextureID));

        const SkyMaterial material = {
                .descriptorSetIndex = db_add_descriptor_set(descriptorSet),
                .octahedralMapIndex = skyboxTextureID
        };

        octaSkyMaterialID = db_add_sky_material(material);
    }
    //============================================================

    //===ENTITY_MESH==============================================
//    {
//        const Transform transform = {.position{2, 0, -8}, .rotation{0, 45, 0}};
//        const LitEntity defaultCube = {
//                .transformIndex = db_add_transform(transform),
//                .meshIndex = cubeID,
//                .materialIndex = cubeLitMaterialID,
//        };
//        db_add_lit_entity(defaultCube);
//    }
    //============================================================

    //===ENTITY_OCTAHEDRON========================================
    {
        const SkyEntity defaultCube = {
                .meshIndex = octahedronID,
                .materialIndex = octaSkyMaterialID,
        };
        db_add_sky_entity(defaultCube);
    }
    //============================================================
}
//======================================================================================================================

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
    primary_camera_entity_create();
    lit_entities_create();
}
//======================================================================================================================
