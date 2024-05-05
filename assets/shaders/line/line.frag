#version 450

//===STAGE IN===============================================
layout (location = 0) in StageLayout {
    vec3 color;
} stageLayout;
//==========================================================

//===OUT====================================================
layout (location = 0) out vec4 outFragColor;
//==========================================================

void main(){
    vec3 col = stageLayout.color;
    vec4 outCol = vec4(col.rgb, 1.0f);
    outFragColor = outCol;
}
