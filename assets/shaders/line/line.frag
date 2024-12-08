#version 450

//===IN=====================================================
layout (location = 0) in StageLayout {
    vec4 color;
} stageLayout;

layout(set = 0, binding = 2) uniform sampler2D depthSampler;
//==========================================================

//===OUT====================================================
layout (location = 0) out vec4 outFragColor;
//==========================================================

void main(){
    const float currentDepth = texture(depthSampler, gl_FragCoord.xy / vec2(textureSize(depthSampler, 0))).r;
    const float fragDepth = gl_FragCoord.z;

    if (stageLayout.color.a == 0.0f){
        if (fragDepth > currentDepth) {
            outFragColor = vec4(stageLayout.color.rgb, 0.3);
        } else {
            outFragColor = vec4(stageLayout.color.rgb, 1.0);
        }
    }
    else {
        outFragColor = stageLayout.color;
    }
}
