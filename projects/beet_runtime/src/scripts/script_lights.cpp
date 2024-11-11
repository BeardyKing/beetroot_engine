#include <runtime/scripts/script_lights.h>

#include <beet_gfx/vulkan/gfx_vk_line.h>

#include <beet_math/vec2.h>
#include <beet_math/vec3.h>

//===API================================================================================================================
void script_create_lights() {
}

constexpr uint32_t SCRIPT_LIGHT_COUNT = 4;

const vec3f lightPositions[SCRIPT_LIGHT_COUNT] = {
        vec3f(10, 40, 10),
        vec3f(-30, 10, 10),
        vec3f(0, 10, 10),
        vec3f(30, 10, 10),
};

const vec3f lightColors[SCRIPT_LIGHT_COUNT] = {
        vec3f(1000, 1000, 1000),
        vec3f(1000, 0, 0),
        vec3f(0, 1000, 0),
        vec3f(0, 0, 1000),
};

void script_update_lights(float deltaTime) {
    for (uint32_t i = 0; i < SCRIPT_LIGHT_COUNT; ++i) {
        {
            const LinePoint3D start = {lightPositions[i], 0xFFFFFF00};
            const LinePoint3D end = {lightPositions[i] + WORLD_UP, 0xFFFFFF00};
            gfx_line_add_segment_immediate(start, end, 1);
        }
        {
            const LinePoint3D start = {lightPositions[i], 0xFFFFFF00};
            const LinePoint3D end = {lightPositions[i] - WORLD_UP, 0xFFFFFF00};
            gfx_line_add_segment_immediate(start, end, 1);
        }
        {
            const LinePoint3D start = {lightPositions[i], 0xFFFFFF00};
            const LinePoint3D end = {lightPositions[i] + WORLD_FORWARD, 0xFFFFFF00};
            gfx_line_add_segment_immediate(start, end, 1);
        }
        {
            const LinePoint3D start = {lightPositions[i], 0xFFFFFF00};
            const LinePoint3D end = {lightPositions[i] - WORLD_FORWARD, 0xFFFFFF00};
            gfx_line_add_segment_immediate(start, end, 1);
        }
        {
            const LinePoint3D start = {lightPositions[i], 0xFFFFFF00};
            const LinePoint3D end = {lightPositions[i] + WORLD_RIGHT, 0xFFFFFF00};
            gfx_line_add_segment_immediate(start, end, 1);
        }
        {
            const LinePoint3D start = {lightPositions[i], 0xFFFFFF00};
            const LinePoint3D end = {lightPositions[i] - WORLD_RIGHT, 0xFFFFFF00};
            gfx_line_add_segment_immediate(start, end, 1);
        }
    }
}

void script_shutdown_lights() {
}
//======================================================================================================================
