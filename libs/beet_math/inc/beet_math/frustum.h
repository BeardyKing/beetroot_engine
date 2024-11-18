#ifndef BEETROOT_FRUSTUM_H
#define BEETROOT_FRUSTUM_H

#include <beet_math/vec3.h>
#include <beet_math/vec2.h>
#include <glm/geometric.hpp>

struct GfxViewFrustum {
    vec3f origin;
    vec3f normal;
    vec3f up;
    vec2f nearSize;
    vec2f farSize;
    float zNear;
    float zFar;
};

struct Frustum {
    vec3f nearTopLeft;
    vec3f nearTopRight;
    vec3f nearBottomLeft;
    vec3f nearBottomRight;
    vec3f farTopLeft;
    vec3f farTopRight;
    vec3f farBottomLeft;
    vec3f farBottomRight;
};

Frustum view_frustum_to_frustum(const GfxViewFrustum &viewFrustum);

#endif //BEETROOT_FRUSTUM_H
