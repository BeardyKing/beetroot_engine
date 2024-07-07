#include <beet_math/mat4.h>
#include <beet_math/vec3.h>

vec3f mat4f_extract_up(const mat4f &matrix) {
    return vec3f(matrix[1]);
}

vec3f mat4f_extract_right(const mat4f &matrix) {
    return vec3f(matrix[0]);
}

vec3f mat4f_extract_forward(const mat4f &matrix) {
    return vec3f(matrix[2]);
}

vec3f mat4f_extract_position(const mat4f &matrix) {
    return vec3f(matrix[3]);
}

vec3f mat4f_extract_scale(const mat4f &matrix) {
    return vec3f(
            glm::length(vec3f(matrix[0])),
            glm::length(vec3f(matrix[1])),
            glm::length(vec3f(matrix[2]))
    );
}

vec3f mat4f_extract_rotation_euler(const mat4f &matrix) {
    const quat rotationQuat = mat4f_extract_rotation_quat(matrix);
    const vec3f eulerAngles = glm::degrees(glm::eulerAngles(rotationQuat));
    return eulerAngles;
}

quat mat4f_extract_rotation_quat(const mat4f &matrix) {
    const vec3f scale = mat4f_extract_scale(matrix);
    const mat3 rotationMatrix = mat3(
            vec3f(matrix[0]) / scale.x,
            vec3f(matrix[1]) / scale.y,
            vec3f(matrix[2]) / scale.z
    );
    return glm::quat_cast(rotationMatrix);
}