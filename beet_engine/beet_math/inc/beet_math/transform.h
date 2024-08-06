#ifndef BEETROOT_TRANSFORM_H
#define BEETROOT_TRANSFORM_H

#include <beet_math/mat4.h>

//===PUBLIC_STRUCTS=====================================================================================================
struct Transform {
    vec3f position{0.0f, 0.0f, 0.0f};
    vec3f rotation{0.0f, 0.0f, 0.0f};
    vec3f scale{1.0f, 1.0f, 1.0f};
};
//======================================================================================================================

//===API================================================================================================================
void transform_rotate(Transform &transform, float angleDegrees, const vec3f &axis, bool worldSpace);
void transform_rotate_local(Transform &transform, float angleDegrees, const vec3f &axis);
void transform_rotate_world(Transform &transform, float angleDegrees, const vec3f &axis);

mat4 transform_model_matrix(const Transform &transform);
mat4 transform_model_matrix_no_rotation(const Transform &transform);
//======================================================================================================================

#endif //BEETROOT_TRANSFORM_H
