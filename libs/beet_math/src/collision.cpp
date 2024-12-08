#include <beet_math/collision.h>

//===API================================================================================================================
vec3f collision_aabb_closest_point(const GfxAABB &box, const vec3f &point) {
    const vec3f closestPoint = (vec3f) {
            fmaxf((box.center.x - box.halfExtents.x), fminf(point.x, box.center.x + box.halfExtents.x)),
            fmaxf((box.center.y - box.halfExtents.y), fminf(point.y, box.center.y + box.halfExtents.y)),
            fmaxf((box.center.z - box.halfExtents.z), fminf(point.z, box.center.z + box.halfExtents.z))
    };
    return closestPoint;
}

bool collision_aabb_sphere(const GfxBox &box, const GfxCircle &circle) {
    const vec3f closestPoint = collision_aabb_closest_point((GfxAABB) {.center = box.center, .halfExtents = box.halfExtents}, circle.center);
    const float distanceSq = vec3f_sq(circle.center - closestPoint);
    return distanceSq <= (circle.radius * circle.radius);
}
//======================================================================================================================