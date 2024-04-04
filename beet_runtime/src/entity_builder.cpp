#include <runtime/entity_builder.h>

#include <beet_shared/beet_types.h>

#include <beet_gfx/gfx_types.h>
#include <beet_gfx/db_asset.h>
#include <beet_gfx/gfx_texture.h>
#include <beet_gfx/gfx_lit.h>


void primary_camera_entity_create() {
    const CameraEntity cameraEntity{
            .transformIndex = db_add_transform({.position{0, 0, -1}}),
            .cameraIndex = db_add_camera({.fov = 65}),
    };
    db_add_camera_entity(cameraEntity);
}

void lit_entities_create() {
    uint32_t defaultMeshID = {UINT32_MAX};
    uint32_t defaultMaterialID = {UINT32_MAX};
    {
        GfxTexture uvTestTexture = {};
        gfx_texture_create_immediate_dds("../res/textures/UV_Grid/UV_Grid_test.dds", uvTestTexture);

        VkDescriptorSet descriptorSet = {VK_NULL_HANDLE};
        gfx_lit_update_material_descriptor(descriptorSet, uvTestTexture);

        const LitMaterial material = {
                .descriptorSetIndex = db_add_descriptor_set(descriptorSet),
                .albedoIndex = db_add_texture(uvTestTexture),
        };

        GfxMesh mesh = {};
        gfx_mesh_create_cube_immediate(mesh);

        defaultMeshID = db_add_mesh(mesh);
        defaultMaterialID = db_add_lit_material(material);

        const Transform transform = {.position{2, 0, -8}};
        const LitEntity defaultCube = {
                .transformIndex = db_add_transform(transform),
                .meshIndex = defaultMeshID,
                .materialIndex = defaultMaterialID,
        };
        db_add_lit_entity(defaultCube);
    }
    {
        const Transform transform = {.position = {-2, 1, -12}};
        const LitEntity defaultCube = {
                .transformIndex = db_add_transform(transform),
                .meshIndex = defaultMeshID,
                .materialIndex = defaultMaterialID,
        };
        db_add_lit_entity(defaultCube);
    }
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