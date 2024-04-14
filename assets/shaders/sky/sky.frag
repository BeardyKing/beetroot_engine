#version 450

//===STAGE_IN===============================================
layout (location = 0) in StageLayout {
    vec3 color;
    vec3 normal;
    vec2 uv;
} stageLayout;
//==========================================================

//===LOCAL==================================================
layout (binding = 1) uniform sampler2D u_albedo;
//==========================================================

//===OUT====================================================
layout (location = 0) out vec4 outFragColor;
//==========================================================

void main(){
    vec2 uv = stageLayout.uv;
    vec4 outCol = vec4(texture(u_albedo, uv).xyz, 1.0f);
    outFragColor = outCol;
}
