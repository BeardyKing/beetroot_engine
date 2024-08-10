#include <beet_math/transform.h>

mat4f transform_model_matrix_position(const Transform &transform) {
    return mat4f{translate(MAT4F_IDENTITY, transform.position)};
}

mat4f transform_model_matrix_rotation(const Transform &transform) {
    return mat4f{translate(MAT4F_IDENTITY, transform.position)};
}

mat4f transform_model_matrix_scale(const Transform &transform) {
    return mat4{scale(MAT4F_IDENTITY, transform.scale)};
}

mat4f transform_model_matrix(const Transform &transform) {
    return mat4f{
            translate(MAT4F_IDENTITY, transform.position) *
            toMat4(quat(radians(transform.rotation))) *
            scale(MAT4F_IDENTITY, transform.scale)
    };
}

mat4f transform_model_matrix_no_rotation(const Transform &transform) {
    return mat4f{
            translate(MAT4F_IDENTITY, transform.position) *
            scale(MAT4F_IDENTITY, transform.scale)
    };
}

mat4f transform_model_matrix_no_scale(const Transform &transform) {
    return mat4{
            translate(MAT4F_IDENTITY, transform.position) *
            toMat4(quat(radians(transform.rotation)))
    };
}

mat4f transform_model_matrix_no_position(const Transform &transform) {
    return mat4{
            toMat4(quat(radians(transform.rotation))) *
            scale(MAT4F_IDENTITY, transform.scale)
    };
}

void transform_rotate_world(Transform &transform, const float angleDegrees, const vec3f &axis) {
    const quat rotationQuat = angleAxis(radians(angleDegrees), normalize(axis));
    const quat currentRotation = rotationQuat * quat(radians(transform.rotation));
    transform.rotation = glm::degrees(glm::eulerAngles(currentRotation));
}

void transform_rotate_local(Transform &transform, const float angleDegrees, const vec3f &axis) {
    const quat rotationQuat = angleAxis(radians(angleDegrees), normalize(axis));
    const quat currentRotation = quat(radians(transform.rotation)) * rotationQuat;
    transform.rotation = degrees(eulerAngles(currentRotation));
}

void transform_rotate(Transform &transform, const float angleDegrees, const vec3f &axis, const bool worldSpace) {
    if (worldSpace) {
        transform_rotate_world(transform, angleDegrees, axis);
    } else {
        transform_rotate_local(transform, angleDegrees, axis);
    }
}