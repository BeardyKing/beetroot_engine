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

void main() {

    const float currentDepth = texture(depthSampler, gl_FragCoord.xy / vec2(textureSize(depthSampler, 0))).r;
    const float fragDepth = gl_FragCoord.z;

    if (stageLayout.color.a == 0.0f) {
        const int gridSize = 5; // world space grid dither size.
        ivec2 gridCoord = ivec2(gl_FragCoord.xy) / gridSize;
        bool ditherDiscard = (gridCoord.x % 2 == gridCoord.y % 2);
        bool skipDither = false; // could consider as shader param
        if (fragDepth > currentDepth) {
            if (!ditherDiscard && !skipDither) {
                discard;
            }
            vec3 col = stageLayout.color.rgb;
            col.r += 0.5;
            col.g += 0.25f;
            col.b -= 1.0f;
            outFragColor = vec4(col, 0.1);
        }
//        else
//        {
//            outFragColor = vec4(stageLayout.color.rgb, 0.3);
//        }
    }
    else{
        outFragColor = stageLayout.color;
    }

}



