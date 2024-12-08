#ifndef BEETROOT_FRUSTUM_H
#define BEETROOT_FRUSTUM_H

#include <beet_math/shapes.h>

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

//===API================================================================================================================
Frustum view_frustum_to_frustum(const GfxViewFrustum &viewFrustum);
//======================================================================================================================

#endif //BEETROOT_FRUSTUM_H
