#ifndef BEETROOT_MAT4_H
#define BEETROOT_MAT4_H

#include <beet_math/vec3.h>
#include <beet_math/quat.h>

#include <glm/mat4x2.hpp>
#include <glm/mat4x3.hpp>
#include <glm/mat4x4.hpp>

#include <glm/gtc/matrix_transform.hpp>

using mat4f = mat<4, 4, float, glm::defaultp>;
constexpr mat4f MAT4F_IDENTITY = glm::identity<mat4f>();

//===API================================================================================================================
vec3f mat4f_extract_up(const mat4f &matrix);
vec3f mat4f_extract_right(const mat4f &matrix);
vec3f mat4f_extract_forward(const mat4f &matrix);

vec3f mat4f_extract_position(const mat4f &matrix);
vec3f mat4f_extract_scale(const mat4f &matrix);
vec3f mat4f_extract_rotation_euler(const mat4f &matrix);
quat mat4f_extract_rotation_quat(const mat4f &matrix);
//======================================================================================================================

#endif //BEETROOT_MAT4_H
