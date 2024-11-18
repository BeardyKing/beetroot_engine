#include <runtime/scripts/script_lights.h>

#include <beet_gfx/vulkan/gfx_vk_line.h>

#include <beet_math/vec2.h>
#include <beet_math/vec3.h>
#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>
#include <beet_math/transform.h>
#include <beet_gfx/db_asset.h>

#include <beet_gfx/gfx_immediate_draw.h>

//===API================================================================================================================
void script_create_lights() {
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{3, 1, 4}}),
            .lightIndex = db_add_light((GfxLight) {.color = {10, 10, 10}, .radius = 10.0f,}),
    });
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{3, 1, 1}}),
            .lightIndex = db_add_light((GfxLight) {.color = {10, 0, 0}, .radius = 10.0f,}),
    });
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{0, 1, 2}}),
            .lightIndex = db_add_light((GfxLight) {.color = {0, 10, 0}, .radius = 10.0f,}),
    });
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{1, 1, 6}}),
            .lightIndex = db_add_light((GfxLight) {.color = {0, 0, 10}, .radius = 10.0f,}),
    });
}


static void script_update_lights(float deltaTime) {
    const uint32_t lightCount = db_get_light_entity_count();
    for (uint32_t i = 0; i < lightCount; ++i) {
        const LightEntity *lightEntity = db_get_light_entity(i);
        const GfxLight *light = db_get_light(lightEntity->lightIndex);
        const Transform *transform = db_get_transform(lightEntity->transformIndex);
        const vec4f v4Colour = vec4f(glm::normalize(light->color), 0);
        const uint32_t lineColour = pack_vec4f_to_uint32_t(v4Colour);
        const vec3f lookDir = transform->position - db_get_transform(db_get_camera_entity(0)->transformIndex)->position;
        gfx_im_draw_line_sun({.center = transform->position, .radius = 0.05f, .normal = lookDir, .up = WORLD_UP}, lineColour);
    }
}

void script_shutdown_lights() {
}
//======================================================================================================================
