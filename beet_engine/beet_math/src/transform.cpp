#include <beet_math/transform.h>

mat4f transform_model_matrix_position(const Transform &transform) {
    return mat4f{translate(MAT4F_IDENTITY, transform.position)};
}

mat4f transform_model_matrix_rotation(const Transform &transform) {
    return mat4f{toMat4(quat(radians(transform.rotation)))};
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

void transform_translate_local(Transform &transform, const vec3f &offset) {
    transform.position += mat4f_extract_position(transform_model_matrix_rotation(transform) * translate(MAT4F_IDENTITY, offset));
}

void transform_translate_world(Transform &transform, const vec3f &offset) {
    transform.position += offset;
}

void transform_translate(Transform &transform, const vec3f &offset, bool worldSpace) {
    if (worldSpace) {
        transform_translate_world(transform, offset);
    } else {
        transform_translate_local(transform, offset);
    }
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

void transform_scale_local(Transform &transform, const vec3f &scale) {
    transform.scale += scale;
}

void transform_scale_world(Transform &transform, const vec3f &scale) {
    const quat rotQuat = quat(transform.rotation);
    const vec3f forward = abs(rotQuat * WORLD_FORWARD);
    const vec3f right = abs(rotQuat * WORLD_RIGHT);
    const vec3f up = abs(rotQuat * WORLD_UP);
    const vec3f scaleContribution =
            scale.x * right +
            scale.y * up +
            scale.z * forward;
    transform.scale += scaleContribution;
}

void transform_scale(Transform &transform, const vec3f &scale, const bool worldSpace) {
    if (worldSpace) {
        transform_scale_world(transform, scale);
    } else {
        transform_scale_local(transform, scale);
    }
}