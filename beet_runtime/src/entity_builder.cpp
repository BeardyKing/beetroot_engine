#include <runtime/entity_builder.h>

#include <beet_shared/beet_types.h>

#include <beet_gfx/gfx_types.h>
#include <beet_gfx/db_asset.h>
#include <beet_gfx/gfx_texture.h>
#include <beet_gfx/gfx_lit.h>
#include <beet_gfx/gfx_sky.h>
#include <beet_gfx/gfx_samplers.h>


void primary_camera_entity_create() {
    const CameraEntity cameraEntity{
            .transformIndex = db_add_transform({.position{0, 0, -1}}),
            .cameraIndex = db_add_camera({.fov = 65}),
    };
    db_add_camera_entity(cameraEntity);
}

void lit_entities_create() {
    //===MESH=====================================================
    uint32_t cubeID = {UINT32_MAX};
    uint32_t octahedronID = {UINT32_MAX};
    {
        GfxMesh cubeMesh = {};
        gfx_mesh_create_cube_immediate(cubeMesh);
        cubeID = db_add_mesh(cubeMesh);

        GfxMesh octahedronMesh = {};
        gfx_mesh_create_octahedron_immediate(octahedronMesh);
        octahedronID = db_add_mesh(octahedronMesh);
    }
    //============================================================

    //===TEXTURE==================================================
    uint32_t uvGridTextureID = {UINT32_MAX};
    {
        GfxTexture uvTestTexture = {};
        gfx_texture_create_immediate_dds("../res/textures/UV_Grid/UV_Grid_test.dds", uvTestTexture);
        uvGridTextureID = db_add_texture(uvTestTexture);
    }
    uint32_t skyboxTextureID = {UINT32_MAX};
    {
        GfxTexture skyboxTexture = {.imageSamplerType = TextureSamplerType::LinearMirror};
        gfx_texture_create_immediate_dds("../res/textures/sky/herkulessaulen_4k-octahedral.dds", skyboxTexture);
        skyboxTextureID = db_add_texture(skyboxTexture);
    }
    //============================================================

    //===MATERIAL=================================================
    uint32_t cubeLitMaterialID = {UINT32_MAX};
    {
        VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
        gfx_lit_update_material_descriptor(descriptorSet, *db_get_texture(uvGridTextureID));

        const LitMaterial material = {
                .descriptorSetIndex = db_add_descriptor_set(descriptorSet),
                .albedoIndex = uvGridTextureID
        };

        cubeLitMaterialID = db_add_lit_material(material);
    }

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

    //===ENTITY_CUBE==============================================
    {
        const Transform transform = {.position{2, 0, -8}};
        const LitEntity defaultCube = {
                .transformIndex = db_add_transform(transform),
                .meshIndex = cubeID,
                .materialIndex = cubeLitMaterialID,
        };
        db_add_lit_entity(defaultCube);
    }
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