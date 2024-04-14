#version 450

//===GLOBAL=================================================
layout (set = 0, binding = 0) uniform SceneUBO {
    mat4 projection;
    mat4 view;
    vec3 position;
    float unused_0;
} scene;
//==========================================================

//===LOCAL==================================================
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_uv;
layout (location = 3) in vec3 v_color;

layout (push_constant) uniform PushConstants {
    mat4 model;
} constants;
//==========================================================

//===STAGE OUT==============================================
layout (location = 0) out StageLayout {
    vec3 color;
    vec3 normal;
    vec2 uv;
} stageLayout;
//==========================================================

void main() {
    gl_Position = (scene.projection * scene.view) * (vec4(v_position, 1.0) + vec4(scene.position.xyz,0.0f));

    stageLayout.color = v_color;
    stageLayout.uv = v_uv;
    stageLayout.normal = v_normal;
}
