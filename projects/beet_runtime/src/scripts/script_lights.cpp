#include <runtime/scripts/script_lights.h>
#include <runtime/widget_db.h>

#include <beet_math/collision.h>
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
            .lightIndex = db_add_light((GfxLight) {.color = {10, 10, 10}, .radiusInner = 1.0f, .radiusOuter = 2.0f,}),
    });
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{3, 1, 1}}),
            .lightIndex = db_add_light((GfxLight) {.color = {10, 0, 0}, .radiusInner = 1.0f, .radiusOuter = 1.2f,}),
    });
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{0, 1, 2}}),
            .lightIndex = db_add_light((GfxLight) {.color = {0, 10, 0}, .radiusInner = 1.0f, .radiusOuter = 1.2f,}),
    });
    db_add_light_entity((LightEntity) {
            .transformIndex = db_add_transform((Transform) {.position{1, 1, 6}}),
            .lightIndex = db_add_light((GfxLight) {.color = {0, 0, 10}, .radiusInner = 0.3f, .radiusOuter = 0.5f,}),
    });
}

constexpr uint16_t GRID_X = 16;
constexpr uint16_t GRID_Y = 8;
constexpr uint16_t GRID_Z = 16;
constexpr uint16_t GRID_COUNT = GRID_X * GRID_Y * GRID_Z;

constexpr uint32_t LIGHTS_PER_CELL = 8;
struct LightCellInfo {
    uint32_t lightIndex[LIGHTS_PER_CELL] = {};
};

struct LightGrid {
    LightCellInfo cell[GRID_COUNT] = {};
    uint32_t cellCount[GRID_COUNT] = {};
};

static LightGrid s_lightGrid = {};

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

        if (poolType == SELECTED_POOL_LIGHT_ENT && i == poolIndex) {
            gfx_im_draw_line_sphere((GfxCircle) {.center = transform->position, .radius = light->radiusInner, .normal = lookDir, .up = WORLD_UP}, lineColour, 64, 2.0f);
            gfx_im_draw_line_sphere((GfxCircle) {.center = transform->position, .radius = light->radiusOuter, .normal = lookDir, .up = WORLD_UP}, lineColour, 64, 2.0f);
        }

        static bool debugFrustumDraw = false;
        if (i == 1 && debugFrustumDraw) {
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

    // light grid
    //TODO: This code should be ported to a compute shader...
    memset(s_lightGrid.cellCount, 0, sizeof(s_lightGrid.cellCount));
    static bool showOverlap = false;
    ImGui::Checkbox("Show light grid overlaps", &showOverlap);

    for (uint32_t voxelIndex = 0; voxelIndex < GRID_COUNT; ++voxelIndex) {
        const uint32_t x = voxelIndex % GRID_X;
        const uint32_t y = (voxelIndex / GRID_X) % GRID_Y;
        const uint32_t z = voxelIndex / (GRID_X * GRID_Y);

        const GfxBox box = {
                .center = {x - (GRID_X / 2.0f), y - (GRID_Y / 2.0f), z - (GRID_Z / 2.0f)},
                .halfExtents = vec3{0.5f},
                .normal = {WORLD_FORWARD},
                .up = {WORLD_UP}
        };

        const uint32_t lightCount = db_get_light_entity_count();
        for (uint32_t lightIndex = 0; lightIndex < lightCount; ++lightIndex) {
            const LightEntity *lightEntity = db_get_light_entity(lightIndex);
            const GfxLight *light = db_get_light(lightEntity->lightIndex);
            const Transform *transform = db_get_transform(lightEntity->transformIndex);
            const vec3f lookDir = transform->position - db_get_transform(db_get_camera_entity(0)->transformIndex)->position;
            GfxCircle lightCircle{.center = transform->position, .radius = light->radiusOuter, .normal = lookDir, .up = WORLD_UP};

            if (collision_aabb_sphere(box, lightCircle)) {
                uint32_t &currentCellCount = s_lightGrid.cellCount[voxelIndex];
                ASSERT(currentCellCount + 1 < LIGHTS_PER_CELL);
                s_lightGrid.cell[voxelIndex].lightIndex[currentCellCount] = lightIndex;
                currentCellCount++;
            }
        }

        if (showOverlap && s_lightGrid.cellCount[voxelIndex]) {
            constexpr vec4f COLOUR_GREEN = vec4f(0.0f, 1.0f, 0.0f, 0.01);
            constexpr vec4f COLOUR_RED = vec4f(1.0f, 0.0f, 0.0f, 0.15f);
            const float t = (float) s_lightGrid.cellCount[voxelIndex] / (float) LIGHTS_PER_CELL;
            gfx_im_draw_poly_box(box, pack_vec4f_to_uint32_t(glm::mix(COLOUR_GREEN, COLOUR_RED, t)));
        }
    }
}

void script_shutdown_lights() {}
//======================================================================================================================
