#version 450

//===GLOBAL=================================================
layout (set = 0, binding = 0) uniform SceneUBO {
    mat4 projection;
    mat4 view;
    vec3 position;
    float unused_0;
} scene;
//==========================================================
//===MUST_MIRROR_gfx_line.h=================================
#define MAX_POINT_SIZE (1024 * 2)
struct LinePoint3D {
    vec3 position;
    uint color;
};
//==========================================================

layout (set = 0, binding = 1) uniform LinesUBO {
    LinePoint3D point[MAX_POINT_SIZE];
} lines;

//===STAGE OUT==============================================
layout (location = 0) out StageLayout {
    vec3 color;
} stageLayout;
//==========================================================

vec4 unpack_uint32_t_to_vec4f(uint packedValue) {
    // Extract each component from the packed uint
    float x = float((packedValue >> 24) & 0xFF) / 255.0;
    float y = float((packedValue >> 16) & 0xFF) / 255.0;
    float z = float((packedValue >> 8)  & 0xFF) / 255.0;
    float w = float((packedValue >> 0)  & 0xFF) / 255.0;

    return vec4(x, y, z, w);
}

void main() {
    vec4 position = vec4(lines.point[gl_VertexIndex].position, 1.0);
    vec4 color = unpack_uint32_t_to_vec4f(lines.point[gl_VertexIndex].color);
    gl_Position = (scene.projection * scene.view) * position;
    stageLayout.color = color.rgb;
}
