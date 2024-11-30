#include <runtime/scripts/script_lights.h>
#include <runtime/widget_db.h>

#include <beet_math/vec2.h>
#include <beet_math/vec3.h>
#include <beet_shared/beet_types.h>
#include <beet_shared/assert.h>
#include <beet_math/transform.h>
#include <beet_gfx/db_asset.h>

#include <beet_gfx/gfx_types.h>
#include <beet_gfx/gfx_immediate_draw.h>
#include "imgui.h"

//===API================================================================================================================
void script_create_lights() {
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{3, 1, 4}}),
            .lightIndex = db_add_light((GfxLight) {.color = {10, 10, 10},.radiusInner = 1.0f, .radiusOuter = 2.0f,}),
    });
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{3, 1, 1}}),
            .lightIndex = db_add_light((GfxLight) {.color = {10, 0, 0},.radiusInner = 1.0f, .radiusOuter = 1.2f,}),
    });
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{0, 1, 2}}),
            .lightIndex = db_add_light((GfxLight) {.color = {0, 10, 0},.radiusInner = 1.0f, .radiusOuter = 1.2f,}),
    });
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{1, 1, 6}}),
            .lightIndex = db_add_light((GfxLight) {.color = {0, 0, 10},.radiusInner = 0.3f, .radiusOuter = 0.5f,}),
    });
}

static void script_update_lights(float deltaTime) {

    const SelectedPool &poolType = *get_selected_pool_type();
    const int32 &poolIndex = *get_selected_pool_index();



    const uint32_t lightCount = db_get_light_entity_count();
    for (uint32_t i = 0; i < lightCount; ++i) {
        const LightEntity *lightEntity = db_get_light_entity(i);
        const GfxLight *light = db_get_light(lightEntity->lightIndex);
        const Transform *transform = db_get_transform(lightEntity->transformIndex);
        const vec4f v4Colour = vec4f(glm::normalize(light->color), 0);
        const uint32_t lineColour = pack_vec4f_to_uint32_t(v4Colour);
        const vec3f lookDir = transform->position - db_get_transform(db_get_camera_entity(0)->transformIndex)->position;
        gfx_im_draw_line_sun({.center = transform->position, .radius = 0.05f, .normal = lookDir, .up = WORLD_UP}, lineColour);

        static vec2f nearSize = vec2f{0.125f, 0.125f};
        static vec2f farSize = vec2f{0.5f, 0.5f};
        static float zNear = 0.1f;
        static float zFar = 1.0;

        if(poolType == SELECTED_POOL_LIGHT_ENT && i == poolIndex)
        {
            gfx_im_draw_line_sphere((GfxCircle){.center = transform->position, .radius = light->radiusInner, .normal = lookDir, .up = WORLD_UP}, lineColour);
            gfx_im_draw_line_sphere((GfxCircle){.center = transform->position, .radius = light->radiusOuter, .normal = lookDir, .up = WORLD_UP}, lineColour);
        }

        static bool debugFrustumDraw = false;
        if (i == 1 && debugFrustumDraw ) {
            ImGui::Begin("frustum debug");
            {
                ImGui::DragFloat2("nearSize", &nearSize[0], 0.01f);
                ImGui::DragFloat2("farSize", &farSize[0], 0.01f);
                ImGui::DragFloat("zNear", &zNear, 0.01f);
                ImGui::DragFloat("zFar", &zFar, 0.01f);
            }
            ImGui::End();
            GfxViewFrustum viewFrustum = {
                    .origin = transform->position,
                    .normal = mat4f_extract_forward(transform_model_matrix_rotation(*transform)),
                    .up = WORLD_UP,
                    .nearSize = nearSize,
                    .farSize = farSize,
                    .zNear =zNear,
                    .zFar = zFar
            };
            gfx_im_draw_line_view_frustum(viewFrustum, 0xFFFF0000, 2);
            gfx_im_draw_poly_frustum(view_frustum_to_frustum(viewFrustum), 0xFFFFFF00);

            GfxBox box = {
                    .center = transform->position,
                    .halfExtents = vec3f(.25f),
                    .normal = mat4f_extract_forward(transform_model_matrix_rotation(*transform)),
                    .up = WORLD_UP,
            };
            gfx_im_draw_poly_box(box, 0xFFFFFF00);
            gfx_im_draw_line_box(box, 0xFFFFFF00, 2);
        }
    }
}

void script_shutdown_lights() {
}
//======================================================================================================================
