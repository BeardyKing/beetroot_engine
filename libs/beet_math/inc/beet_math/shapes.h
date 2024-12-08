#ifndef BEETROOT_SHAPES_H
#define BEETROOT_SHAPES_H

#include <beet_math/vec3.h>
#include <beet_math/vec2.h>

struct GfxRect {
    vec3f center;
    vec2f halfExtents;
    vec3f normal;
    vec3f up;
};

struct GfxAABB{
    vec3f center;
    vec3f halfExtents;
};

struct GfxBox{
    vec3f center;
    vec3f halfExtents;
    vec3f normal;
    vec3f up;
};

struct GfxCircle {
    vec3f center;
    float radius;
    vec3f normal;
    vec3f up;
};

struct GfxViewFrustum {
    vec3f origin;
    vec3f normal;
    vec3f up;
    vec2f nearSize;
    vec2f farSize;
    float zNear;
    float zFar;
};

#endif //BEETROOT_SHAPES_H
