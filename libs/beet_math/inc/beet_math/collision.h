#ifndef BEETROOT_COLLISION_H
#define BEETROOT_COLLISION_H

#include <beet_math/shapes.h>

//===API================================================================================================================
bool collision_aabb_sphere(const GfxBox &box, const GfxCircle &circle);
vec3f collision_aabb_closest_point(const GfxAABB &box, const vec3f &point);
//======================================================================================================================
#endif //BEETROOT_COLLISION_H
