#ifndef BEETROOT_TRANSFORM_H
#define BEETROOT_TRANSFORM_H

#include <beet_math/mat4.h>
#include <beet_math/vec3.h>

//===PUBLIC_STRUCTS=====================================================================================================
struct Transform {
    vec3f position{0.0f, 0.0f, 0.0f};
    vec3f rotation{0.0f, 0.0f, 0.0f};
    vec3f scale{1.0f, 1.0f, 1.0f};
};
//======================================================================================================================

//===API================================================================================================================
void transform_translate(Transform &transform, const vec3f &offset, bool worldSpace);
void transform_translate_local(Transform &transform, const vec3f &offset);
void transform_translate_world(Transform &transform, const vec3f &offset);

void transform_rotate(Transform &transform, float angleDegrees, const vec3f &axis, bool worldSpace);
void transform_rotate_local(Transform &transform, float angleDegrees, const vec3f &axis);
void transform_rotate_world(Transform &transform, float angleDegrees, const vec3f &axis);

void transform_scale(Transform &transform, const vec3f &scale, bool worldSpace);
void transform_scale_local(Transform &transform, const vec3f &scale);
void transform_scale_world(Transform &transform, const vec3f &scale);

mat4f transform_model_matrix(const Transform &transform);
mat4f transform_model_matrix_position(const Transform &transform);
mat4f transform_model_matrix_rotation(const Transform &transform);
mat4f transform_model_matrix_scale(const Transform &transform);
mat4f transform_model_matrix_no_position(const Transform &transform);
mat4f transform_model_matrix_no_rotation(const Transform &transform);
mat4f transform_model_matrix_no_scale(const Transform &transform);
//======================================================================================================================

#endif //BEETROOT_TRANSFORM_H
